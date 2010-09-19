clear all; clc;

% Fixed point unit characteristics

Q1616 = fimath('RoundMode', 'nearest', ...
    'OverflowMode', 'saturate', ...
    'ProductMode', 'SpecifyPrecision', ...
    'ProductWordLength', 32, ...
    'ProductFractionLength', 16, ...
    'MaxProductWordLength', 32, ...
    'SumMode', 'SpecifyPrecision', ...
    'SumWordLength', 32, ...
    'SumFractionLength', 16, ...
    'MaxSumWordLength', 32);

% RTTY characteristics
Fmark = 1275.0;
Fspace = 1445.0;
F0 = (Fmark + Fspace) / 2;

Wmark = 2 * pi() * Fmark;
Wspace = 2 * pi() * Fspace;

%Generate input signal
dt = 1 / 16000;
sig_vals = round((2.0 ^ 12) * gen_rtty(1 / dt, Fspace, Fmark, 1 / 70, '01'));
sig_time = (0 : length(sig_vals) - 1) * dt;
runtime = sig_time(length(sig_time));

% System characteristics
psi = pi() / 3

W0 = (Wmark + Wspace) / 2;
T0 = 2 * pi() / W0;
D0 = fi((1 / dt) * T0, Q1616);

tau = psi / W0;

W1 = Wmark / W0
W2 = Wspace / W0

K1 = 1;
G1 = K1 / W0;
D1 = fi((1 / dt) * G1, Q1616);

% Loop initialization
t = tau;
kftau = fi((1 / dt) * tau, Q1616);
ktau = single(floor(kftau));
kf = kftau + 1;

t_plot = [];
e_plot = [];

while kf < length(sig_vals)
    % Perform sampling
    %x = interp1(sig_time, sig_vals, t - tau);
    %y = interp1(sig_time, sig_vals, t);
    k = single(floor(kf));
    alpha = kf - fi(k, Q1616);
    
    kfx = kf - kftau;
    kx = single(floor(kfx));
    kx_alpha = kfx - fi(kx, Q1616);
    
    x = single(round(...
        (1 - kx_alpha) * sig_vals(kx) + ...
        kx_alpha * sig_vals(kx + 1) ...
        ));
    
    ky = single(floor(kf));
    ky_alpha = kf - fi(ky, Q1616);
    
    y = single(round(...
        (1 - ky_alpha) * sig_vals(ky) + ...
        ky_alpha * sig_vals(ky + 1) ...
        ));
    
    % Phase detector
    e = fi(atan2_lookup(x, y), Q1616);
    %e = fi(atan2(single(x), single(y)), Q1616);
    
    % Digital filter (put in a single pole?)
    cd = D1 * e;
    
    % Time update -- We use fractional indices
    kf = kf + D0 - cd;
    t = dt * single(kf);
    
    % Collect values for plotting
    t_plot = [t_plot t];
    e_plot = [e_plot e];
end

subplot(2, 1, 1);
hold off;
plot(0 : dt : runtime, interp1(sig_time, sig_vals, 0 : dt : runtime));
hold on;
stem(t_plot, interp1(sig_time, sig_vals, t_plot), '*r');

subplot(2, 1, 2);
plot(t_plot, e_plot);
