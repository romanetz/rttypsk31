clear all; clc;

alpha = 400;
Ts = 1 / 8000;

beta = exp(-alpha * Ts);

csys = tf([alpha], [1 alpha]);
%dsys = tf([1 -1], [1 -beta], Ts);
dsys = tf([alpha*Ts], [1 -beta], Ts);

csys_f = @(f) freqresp(csys, f);
dsys_f = @(f) freqresp(dsys, f);

f = linspace(0, 2000, 2000);

subplot(3, 1, 1);
hold off;
cr = csys_f(f);
plot(f, abs(cr(:)), 'r-');
hold on;
dr = dsys_f(f);
plot(f, abs(dr(:)), 'b-');
xlabel('Frequency (hz)');
ylabel('Magnitude of Response');
legend('Continuous', 'Discrete');
title('Magnitude of Response of lowpass filters');

subplot(3, 1, 2);
hold off;
cr = csys_f(f);
plot(f, angle(cr(:)), 'r-');
hold on;
dr = dsys_f(f);
plot(f, angle(dr(:)), 'b-');
xlabel('Frequency (hz)');
ylabel('Phase of Response');
legend('Continuous', 'Discrete');
title('Phase of Response of lowpass filters');

t = 0:Ts:1/50;
subplot(3, 1, 3);
hold off;
ct = step(csys, t);
plot(t, ct, 'r-');
hold on;
dt = step(dsys, t);
stem(t, dt, 'b-');
plot(t, max(dt) * 0.2, 'g-');
xlabel('Time (s)');
ylabel('Response');
legend('Continuous', 'Discrete', sprintf('n = %f', (log(0.2) / log(beta) + 1) * Ts));
title('Step Response of lowpass filters');

[num den] = tfdata(dsys);

num = num{1};
den = den{1};

message = 'X coeffs: ';
for i = 1 : length(num)
   message = [message sprintf('[n - %d] (%f, 0x%s), ', i - 1, num(i), dec2hexs(round(2^15 * num(i))))]; 
end

disp(message);

message = 'Y coeffs: ';
for i = 1 : length(den)
   message = [message sprintf('[n - %d] (%f, 0x%s), ', i - 1, den(i), dec2hexs(round(2^15 * den(i))))]; 
end

disp(message);