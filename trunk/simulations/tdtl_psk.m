clear all; clc;

% RTTY characteristics
F0 = 1000;
deltap = pi;

%Generate input signal
dt = 1 / 16000;
sig_vals = gen_psk(1 / dt, F0, deltap, 1 / 70, '010101');
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

t = tau;

while t < runtime
    % Perform sampling
    x = interp1(sig_time, sig_vals, t - tau);
    y = interp1(sig_time, sig_vals, t);
    
    % Phase detector -- Scaling will be internal to tables on proc
    e = atan2(x, y);
    
    % Digital filter (put in a single pole?)
    c = G1 * e;
    
    % Time update
    t = t + (T0 - c);
    
    % Collect values for plotting
    t_plot = [t_plot t];
    e_plot = [e_plot e];
    m_plot = [m_plot m];
end

%subplot(2, 1, 1);
%hold off;
%plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
%hold on;
%stem(t_plot, interp1(sig_time, sig_vals, t_plot), '*r');

%subplot(2, 1, 2);
%hold off;
stem(t_plot, abs(e_plot));
xlabel('Time (s)');
ylabel('Magnitude of Error Signal');
title('Detection of 70hz Phase Reversals on 1000hz Signal');
%hold on;
%stem(t_plot, m_plot, 'r');
