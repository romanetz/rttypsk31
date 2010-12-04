function gen_signal_for_mplab_sim(T, F, filename, sn)
    file = fopen(filename, 'w');

    scale = 2^11 - 1;
    
    Fs = 8000;
    Ts = 1 / Fs;
    
    t = 0 : Ts : T - Ts;
    
    signal = awgn(sin(2 * pi() * F * t), sn);
    
    rtty = round(scale * (signal / max(signal) + 1));
    
    for i = 1 : length(rtty)
       fprintf(file, '0x%X\n', rtty(i)); 
    end
    
    fclose(file);
end