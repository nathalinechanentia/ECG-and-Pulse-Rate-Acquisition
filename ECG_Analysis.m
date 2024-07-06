% For submission
%% Define the file path and sheet details
filename = "Lab2.csv";
rawECG1 = readmatrix(filename);
%% Sample Data
sample = rawECG1(20000:80000);
sample = sample / 1023 * 5;
%% Spectral Analyis: Fourier Transformation
fs = 2000; % Hz
n = length(rawECG1);
f_range = (0:n-1)*fs/n ;
fourier = fft(rawECG1);

%% Spectral Analyis: Plotting the Result
% Plot the Frequency
figure(1)
sgtitle("Spectral Analysis")
subplot(3,1,1)
plot(f_range, abs(fourier))
title("Overall range of frequency")
ylim([0, 200000])
xlim([0, 100])
xlabel("Frequency [Hz]")
ylabel("Amplitude")
subplot(3,1,2)
plot(f_range, abs(fourier))
title("Frequency range below 10 Hz")
ylim([0, 400000])
xlim([0, 10])
xlabel("Frequency [Hz]")
ylabel("Amplitude")
subplot(3,1,3)
plot(f_range, abs(fourier))
title("Frequency range below 2 Hz")
ylim([0, 400000])
xlim([0, 2])
xlabel("Frequency [Hz]")
ylabel("Amplitude")
%% Obtain Sample Signal
fs = 2000;
[Clean_signal, Time_vector] = Signal_Processing(sample, fs, 0);
%% Plot Raw Signal vs Processed Signal Comparison
figure(2)
sgtitle("Representative Signal")
subplot(4,1,1)
plot(Time_vector, sample)
title("Raw Signal: 30-second tranch")
xlabel("Time [s]")
ylabel("Voltage [V]")

subplot(4,1,2)
plot(Time_vector, Clean_signal)
title("Processed Signal: 30-second tranch")
xlabel("Time [s]")
ylabel("Amplitude")

subplot(4,1,3)
plot(Time_vector(20001:40000), sample(20001:40000))
title("Raw Signal: 10-second tranch")
xlabel("Time [s]")
ylabel("Voltage [V]")

subplot(4,1,4)
plot(Time_vector(20001:40000), Clean_signal(20001:40000))
title("Processed Signal: 10-second tranch")
xlabel("Time [s]")
ylabel("Amplitude")
%% Data Analysis: Signal Processing
fs = 2000; % Hz
convert = 1; % Yes, convert from 1023 to 5 V
[Signal, Time] = Signal_Processing(rawECG1, fs, 1);
%% Data Analysis: Signal Plotting
Time_factor = 60000; % 30 seconds for each trace
Delay_constant = 20001;
figure(3)
sgtitle("10-second intervals from each trace")
for Trace = 1:5
    Signal_section = Signal(Delay_constant+(Trace-1)*Time_factor:Delay_constant+Trace*Time_factor);
    Time_section = Time(Delay_constant+(Trace-1)*Time_factor:Delay_constant+Trace*Time_factor);
    subplot(5,1,Trace)
    plot(Time_section(1:20000), Signal_section(1:20000)) % 10-second plotting
    xlabel("Time [s]")
    ylabel("Amplitude")
    
end
% Count the peak and compute the result manually
