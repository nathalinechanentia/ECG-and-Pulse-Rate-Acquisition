/*
 * @brief AD8232 ECG Sensor - ECG Recording and Heartbeat Rate Monitoring
 * 
 * This Arduino sketch implements a QRS complex detection algorithm for ECG signal processing.
 * It uses adaptive thresholding to detect QRS complexes in real-time ECG data.
 * The detected QRS complexes are used to calculate the heartbeat rate (BPM).
 * 
 * The sketch includes the necessary libraries and defines constants for the algorithm.
 * The main loop reads ECG data, performs QRS detection, calculates and updates the BPM value.
 * 
 * The QRS detection algorithm consists of three steps: high-pass filtering, low-pass filtering, and adaptive thresholding.
 * The algorithm uses circular buffers to store ECG data points for filtering and evaluation.
 * 
 * ABYLOVA Bermet (56085298)
 * AITKULOVA Dariia (55532032)
 * CHANENTIA Flavia Nathaline (56726198)
 * KLEDKRUEMAS Jeeraphat (57080079)
*/
 
//  Libraries to be used
#include <stdio.h>
#include <stdlib.h>

#define M       5                                 // Size of ecgBuffer
#define N       30                                // Size of hpBuffer
#define WIN_SIZE    250                           // Size of the sliding window used for adaptive thresholding
#define HP_CONSTANT   ((float) 1 / (float) M)

// Resolution of Random Number Generation (RNG)
#define RAND_RES 100000000

// set pin numbers
const int ECG_PIN = 0;                  // Number of the ECG pin (analog)
const int LEADS_OFF_PLUS_PIN  = 8;      // Number of the LO+ pin (digital)
const int LEADS_OFF_MINUS_PIN = 9;      // Number of the LO- pin (digital) 

// timing variables 
unsigned long previousMicros = 0;         // Store previous time
unsigned long currentMicros = 0;          // Current time
unsigned long foundMicros = 0;            // Time at which last QRS was found 
unsigned long previousFoundMicros = 0;    // Time at which QRS before last was found 

// Minimum time interval (in microseconds) between executions
const long PERIOD = 1000000 / WIN_SIZE;

void setup() {
  // Set digital pins as input and output 
  pinMode(ECG_PIN, OUTPUT);
  pinMode(LEADS_OFF_PLUS_PIN, INPUT); // Setup for leads off detection LO+ 
  pinMode(LEADS_OFF_MINUS_PIN, INPUT); // Setup for leads off detection LO-

  Serial.begin(9600);   // initialize serial communication
}

void loop() {
  // Current time
  currentMicros = micros();

  if (currentMicros - previousMicros >= PERIOD){
    previousMicros = currentMicros;

    // Only read data and perform detection if leads are on
    boolean qrsDetected = false;
    
    // Only read data if ECG sensor has detected that leads are attached to patient
    boolean leads_are_attached = (digitalRead(LEADS_OFF_PLUS_PIN) == 0) && (digitalRead(LEADS_OFF_MINUS_PIN) == 0);
    
    if(leads_are_attached){           
      // Read next ECG data point
      int nextEcgPoint = analogRead(ECG_PIN);
      // Serial.println(nextEcgPoint);
      
      // Give next data point to algorithm
      qrsDetected = detect(nextEcgPoint);

      // If the QRS wave is detected, run the algorithm to derive the current heartbeat rate
      if(qrsDetected == true){
        foundMicros = micros();
    
        // Count the heartbeat rate in beats per minute (bpm)
        float current_bpm = (60.0 / (((float) (foundMicros - previousFoundMicros)) / 1000000.0));
        // Serial.print ("ECG BPM = ");
        Serial.println(current_bpm);  

        previousFoundMicros = foundMicros;

      }
    }
  }
}

// ====== QRS Detection Algorithm ======
// STEPS: 
// 1. High-pass Filtering
// 2. Non-Linear Low-pass Filtering
// 3. Adaptive Thresholding -> Decision-Making

// Circular buffer for input ECG signal 
// Keeps a history of M + 1 samples for high-pass filter 
float ecgBuffer[M + 1] = {0};
int ecgBufferWriteIndex = 0;
int ecgBufferReadIndex = 0;

// Circular buffer for input ECG signal
// Keeps a history of N+1 samples for low-pass filter
float hpBuffer[N + 1] = {0};
int hpBufferWriteIndex = 0;
int hpBufferReadIndex = 0;

// Low-pass filter outputs a single point for every input point
// Goes straight to adaptive filtering for evaluation
float nextEvalPoint = 0;

// Running sums for high-pass and low-pass filters
float hpSum = 0;
float lpSum = 0;

// Working variables for adaptive thresholding
float threshold = 0;
boolean triggered = false;
int trigTime = 0;
float winMax = 0;
int winIndex = 0;

// Number of starting iterations, used to determine when moving windows are filled
int iter_no = 0;

int tmp = 0;

/*
 * @brief Detects QRS complex in ECG signal.
 * 
 * This function takes a new ECG data point and performs QRS complex detection using adaptive thresholding.
 * It returns true if a QRS complex is detected, otherwise it returns false.
 * 
 * @param newEcgPoint The new ECG data point to be processed.
 * @return true if a QRS complex is detected, false otherwise.
 */
 
boolean detect(float newEcgPoint){
  // Copy the new data point into the circular buffer and increment the write index
  ecgBuffer[ecgBufferWriteIndex++] = newEcgPoint;  
  ecgBufferWriteIndex %= (M+1);
 
  /* High-pass filtering */
  if(iter_no < M){
    // Fill the buffer with enough points for the high-pass filter to function
    hpSum += ecgBuffer[ecgBufferReadIndex];
    // Initialize the output buffer with zeros as the filtered output is not yet available
    hpBuffer[hpBufferWriteIndex] = 0;
  }
  else{
    // Update the high-pass sum by adding the new data point and subtracting the oldest data point
    hpSum += ecgBuffer[ecgBufferReadIndex];
    
    tmp = ecgBufferReadIndex - M;
    if(tmp < 0) tmp += M + 1;
    
    hpSum -= ecgBuffer[tmp];
    
    // Calculate the high-pass filtered output using the difference between the current and old data points
    float y1 = 0;
    float y2 = 0;
    
    tmp = (ecgBufferReadIndex - ((M+1)/2));
    if(tmp < 0) tmp += M + 1;
    
    y2 = ecgBuffer[tmp];
    
    y1 = HP_CONSTANT * hpSum;
    
    // Store the high-pass filtered output in the buffer
    hpBuffer[hpBufferWriteIndex] = y2 - y1;
  }

  // Increment the read index of the ECG buffere
  ecgBufferReadIndex++;
  ecgBufferReadIndex %= (M+1);
  
  // Increment the write index of the high-pass filter
  hpBufferWriteIndex++;
  hpBufferWriteIndex %= (N+1);
  
  /* Low-pass filtering */
  // Shift in the new sample from the high-pass filter
  lpSum += hpBuffer[hpBufferReadIndex] * hpBuffer[hpBufferReadIndex];
  
  if(iter_no < N){
    // Fill the buffer with enough points for the low-pass filter to function
    nextEvalPoint = 0;
  }
  else{
    // Shift out the oldest data point
    tmp = hpBufferReadIndex - N;
    if(tmp < 0) tmp += (N+1);
    
    // Subtract the squared value of the oldest data point from the low-pass sum
    lpSum -= hpBuffer[tmp] * hpBuffer[tmp];
    
    // Set the next evaluation point as the updated low-pass sum
    nextEvalPoint = lpSum;
  }
  
  // Increment the read index of the high-pass buffer
  hpBufferReadIndex++;
  hpBufferReadIndex %= (N+1);
  
  /* Adapative thresholding beat detection */
  // Set the initial threshold        
  if(iter_no < WIN_SIZE) {
    if(nextEvalPoint > threshold) {
      threshold = nextEvalPoint;
    }
    // Only increment iter_no if it is less than WIN_SIZE
    iter_no++;
  }

  // Check if the detection hold off period has passed
  if(triggered == true){
    trigTime++;
    
    if(trigTime >= 100){
      triggered = false;
      trigTime = 0;
    }
  }
  
  // Find if there is a new maximum value
  if(nextEvalPoint > winMax) winMax = nextEvalPoint;
  
  // Check if it is above the adaptive threshold and not triggered
  if(nextEvalPoint > threshold && !triggered) {
    triggered = true;

    return true;
  }
  // Finish the function before returning false to potentially change the threshold
          
  // Adjust the adaptive threshold using the maximum value of the signal found in the previous window 
  if(winIndex++ >= WIN_SIZE){
    // Weighting factor: determines the contribution of the current peak value to the threshold adjustment
    float gamma = 0.175;
    // Forgetting factor: rate at which old observations are forgotten
    // A random value between 0.01 and 0.1 is chosen
    float alpha = 0.01 + ( ((float) random(0, RAND_RES) / (float) (RAND_RES)) * ((0.1 - 0.01)));
    
    // Compute new threshold
    threshold = alpha * gamma * winMax + (1 - alpha) * threshold;
    
    // Reset the current window index
    winIndex = 0;
    winMax = -10000000;
  }
      
  // Return false if a new QRS is not detected
  return false;
}
