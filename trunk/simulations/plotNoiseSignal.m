clear all; clear;

Ts = 1 / 2400;

sn = 0;
signal = gen_rtty(1 / Ts, 400, 300, 1 / 100, 2, 'HHHH');
noiseSignal = awgn(signal, sn);

Signal = fftshift(fft(signal));
NoiseSignal = fftshift(fft(noiseSignal));

fbin = (1 / Ts) / length(Signal);
f = (-length(Signal) / 2 : length(Signal) / 2 - 1) * fbin;

figure(1);
subplot(2, 1, 1);
plot(Ts * (0 : length(signal) - 1), signal);
title('Signal');
xlabel('Time (s)');
ylabel('Level');

subplot(2, 1, 2);
plot(Ts * (0 : length(signal) - 1), noiseSignal);
title('Signal with Noise');
xlabel('Time (s)');
ylabel('Level');

figure(2);
subplot(2, 1, 1);
plot(f, abs(Signal));
title('Spectrum of Signal');
xlabel('Frequency (hz)');
ylabel('Magnitude of Spectrum');

subplot(2, 1, 2);
plot(f, abs(NoiseSignal));
title('Spectrum of Signal with Noise');
xlabel('Frequency (hz)');
ylabel('Magnitude of Spectrum');
