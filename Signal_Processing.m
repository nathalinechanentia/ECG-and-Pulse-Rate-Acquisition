function [ECGSignal_Clean, time_vector] = Signal_Processing(InputSignal, fs, convert)

% Convert ADC values to voltage (0-1023 corresponds to 0-5V)
if convert == "Yes"
    ECGSignal = (InputSignal / 1023) * 5;
else
    ECGSignal = InputSignal;
end

% Create time vector for plotting (based on the sampling frequency)
time_vector = (0:length(ECGSignal)-1) / fs;


%====== ECG SIGNAL FILTERING ===================

% High-pass filter to remove baseline wander
hp_cutoff = 0.5; % Cutoff frequency in Hz
[b_hp, a_hp] = butter(4, hp_cutoff/(fs/2), 'high');
ECGSignal_HP = filtfilt(b_hp, a_hp, ECGSignal);

% Bandstop filter to remove power line interference
notch_freq = 50;  % Notch frequency in Hz 
notch_bw = 5;     % Bandwidth around the notch frequency to be attenuated
[b_bs, a_bs] = butter(2, [(notch_freq - notch_bw/2)/(fs/2), (notch_freq + notch_bw/2)/(fs/2)], 'stop');
ECGSignal_HP_BS = filtfilt(b_bs, a_bs, ECGSignal_HP);

% Low-pass filter to remove high-frequency noise
lp_cutoff = 20; % Cutoff frequency in Hz
[b_lp, a_lp] = butter(4, lp_cutoff/(fs/2));
ECGSignal_Clean = filtfilt(b_lp, a_lp, ECGSignal_HP_BS);

%==============================================