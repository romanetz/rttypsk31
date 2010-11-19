function gen_rtty_for_mplab_sim(message, filename, sn)
    file = fopen(filename, 'w');

    scale = 2^11 - 1;
    
    signal = awgn(gen_rtty(8000, 1445, 1275, 1 / 45.45, 2, message), sn);
    
    rtty = round(scale * (signal / max(signal) + 1));
    
    for i = 1 : length(rtty)
       fprintf(file, '0x%X\n', rtty(i)); 
    end
    
    fclose(file);
end