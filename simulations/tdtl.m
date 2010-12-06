clear all; clc;

% RTTY characteristics
Fmark = 200.0;
Fspace = 400.0;
F0 = (Fmark + Fspace) / 2;

Wmark = 2 * pi() * Fmark;
Wspace = 2 * pi() * Fspace;

%Generate input signal
dt = 1 / 8000;
sig_vals = gen_rtty(1 / dt, Fmark, Fspace, 1 / 70, 2, 'H');
sig_time = (0 : length(sig_vals) - 1) * dt;
runtime = sig_time(length(sig_time));

% System characteristics
psi = pi() / 2;

W0 = (Wmark + Wspace) / 2;
T0 = 2 * pi() / W0;

tau = psi / W0;

W1 = Wmark / W0;
W2 = Wspace / W0;

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
    
    %e = x / y;
    %if(e > pi())
    %    e = pi();
    %elseif(e < -pi())
    %    e = -pi();
    %end
    
    % Digital filter (put in a single pole?)
    c = G1 * e;
    
    % Time update
    t = t + (T0 - c);
    
    % Collect values for plotting
    t_plot = [t_plot t];
    e_plot = [e_plot e];
end

h = subplot(2, 1, 1);
plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
xlabel('Time (s)');
ylabel('Signal');
title('BFSK Signal');
axis([0 runtime -1.1 1.1]);

subplot(2, 1, 2);
plot(t_plot, e_plot, 'ob-');
xlabel('Time (s)');
ylabel('Error Signal');
title('Detection of BFSK Signal');
axis([0 runtime min(e_plot) * 1.1 max(e_plot) * 1.1]);

