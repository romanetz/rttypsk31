clear all; clc;

% RTTY characteristics
F0 = 1000;
deltap = pi;

%Generate input signal
dt = 1 / 8000;
sig_vals = gen_psk(1 / dt, F0, 't');
sig_time = (0 : length(sig_vals) - 1) * dt;
runtime = sig_time(length(sig_time));

% System characteristics
psi = pi() / 2;

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

subplot(2, 1, 1);
plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
xlabel('Time (s)');
ylabel('Signal');
title('BPSK Signal');
axis([0 runtime -1.1 1.1]);

subplot(2, 1, 2);
stem(t_plot, abs(e_plot));
xlabel('Time (s)');
ylabel('abs(Error)');
title('Detection of BPSK Signal');
axis([0 runtime -0.3 max(abs(e_plot))*1.1]);
