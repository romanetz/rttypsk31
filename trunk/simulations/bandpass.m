clear all; clc;

%zeta = 0.04 * [(1445/1275), 1];
%wn = [2 * pi() * 1275, 2 * pi() * 1445];
zeta = 0.05;
wn = [2 * pi() * (1275 + 1445) / 2];
wr = wn .* sqrt(1 - zeta.^2);

Ts = 1 / 8000;
alpha = exp(-zeta .* wn * Ts);
w0 = wr * Ts;

i = 1;
csys = tf([wn(i) * (1 - zeta(i)^2)], [1 2 * zeta(i) * wn(i) wn(i)^2]);
dsys = tf([Ts * alpha(i) * sin(w0(i)) 0], [1 -2 * alpha(i) * cos(w0(i)) alpha(i)^2], Ts);
    
for i = 2 : length(wn)
    csys = csys + tf([wn(i) * (1 - zeta(i)^2)], [1 2 * zeta(i) * wn(i) wn(i)^2]);
    dsys = dsys + tf([Ts * alpha(i) * sin(w0(i)) 0], [1 -2 * alpha(i) * cos(w0(i)) alpha(i)^2], Ts);
end

csys_f = @(w) freqresp(csys, w);
dsys_f = @(w) freqresp(dsys, w);

w = 2 * pi() * linspace(0, 2000, 2000);

subplot(2, 1, 1);
hold off;
cr = csys_f(w);
plot(w / (2 * pi()), abs(cr(:)), 'r-');
hold on;
dr = dsys_f(w);
plot(w / (2 * pi()), abs(dr(:)), 'b-');
xlabel('Frequency (hz)');
ylabel('Magnitude of Response');
legend('Continuous', 'Discrete');
title('Magnitude of Response of highpass filters');

subplot(2, 1, 2);
hold off;
cr = csys_f(w);
plot(w / (2 * pi()), angle(cr(:)), 'r-');
hold on;
dr = dsys_f(w);
plot(w / (2 * pi()), angle(dr(:)), 'b-');
xlabel('Frequency (hz)');
ylabel('Phase of Response');
legend('Continuous', 'Discrete');
title('Phase of Response of highpass filters');

%t = 0:Ts:1/50;
%subplot(2, 2, 3);
%rlocus(csys);
%subplot(2, 2, 4);
%rlocus(dsys);


[num den] = tfdata(dsys);

num = num{1};
den = den{1};

num = 100 * num / max(abs(den));
den = den / max(abs(den));

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