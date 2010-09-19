clear all; clc;

% RTTY characteristics
F0 = 1000;
deltap = pi;

%Generate input signal
dt = 1 / 16000;
sig_vals = gen_psk(1 / dt, F0, deltap, 1 / 70, '00010101');
sig_time = (0 : length(sig_vals) - 1) * dt;
runtime = sig_time(length(sig_time));

% System characteristics
psi = pi() / 2

W0 = 2 * pi() * F0;
T0 = 2 * pi() / W0;
D0 = round(T0 * (1 / dt));

tau = psi / W0;

K1 = 1;
G1 = K1 / W0;

D1 = round((2 * pi() / dt) * G1);

% Loop initialization
ktau = round((1 / dt) * tau);
k = ktau + 1;
m = 0;

t_plot = [];
e_plot = [];
m_plot = [];

while k < length(sig_vals)
    % Perform sampling
    %x = (2.0 ^ 12) * interp1(sig_time, sig_vals, t - tau);
    %y = (2.0 ^ 12) * interp1(sig_time, sig_vals, t);
    x = round((2.0 ^ 12) * sig_vals(k - ktau));
    y = round((2.0 ^ 12) * sig_vals(k));
    
    % Phase detector -- Scaling will be internal to tables on proc
    e = (2.0 ^ 15) * atan2_lookup(x, y) / pi();
    
    % Digital filter (put in a single pole?)
    cd = D1 * e;
    
    % Time update
    % t = t + (T0 - c);
    k = k + D0 - cd;
    t = dt * k;
    
    % Collect values for plotting
    t_plot = [t_plot t];
    e_plot = [e_plot e];
    m_plot = [m_plot m];
end

subplot(2, 1, 1);
%hold off;
%plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
%hold on;
stem(t_plot, interp1(sig_time, sig_vals, t_plot), '*r');

subplot(2, 1, 2);
hold off;
plot(t_plot, e_plot);
hold on;
plot(t_plot, m_plot, 'r');
