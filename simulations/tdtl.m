clear all; clc;

% RTTY characteristics
Fmark = 1275.0;
Fspace = 1445.0;
F0 = (Fmark + Fspace) / 2;

Wmark = 2 * pi() * Fmark;
Wspace = 2 * pi() * Fspace;

%Generate input signal
dt = 1 / 16000;
sig_vals = gen_rtty(1 / dt, Fspace, Fmark, 1 / 70, '01');
sig_time = (0 : length(sig_vals) - 1) * dt;
runtime = sig_time(length(sig_time));

% System characteristics
psi = pi() / 3

W0 = (Wmark + Wspace) / 2;
T0 = 2 * pi() / W0;

tau = psi / W0;

W1 = Wmark / W0
W2 = Wspace / W0

K1 = 1;
G1 = K1 / W0;

% Loop initialization
t = tau;

t_plot = [];
e_plot = [];

while t < runtime
    % Perform sampling
    x = interp1(sig_time, sig_vals, t - tau);
    y = interp1(sig_time, sig_vals, t);
    
    % Phase detector
    e = atan2(x, y);
    
    % Digital filter (put in a single pole?)
    c = G1 * e;
    
    % Time update
    t = t + (T0 - c);
    
    % Collect values for plotting
    t_plot = [t_plot t];
    e_plot = [e_plot e];
end

h = subplot(2, 1, 1);
hold off;
plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
hold on;
stem(t_plot, interp1(sig_time, sig_vals, t_plot), '*r');

subplot(2, 1, 2);
plot(t_plot, e_plot);
