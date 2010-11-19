function gen_psk31_for_mplab_sim(message, filename, sn)
    file = fopen(filename, 'w');

    scale = 2^11 - 1;
    
    signal = awgn(gen_psk(8000, 1300, message), sn);
    
    psk31 = round(scale * (signal / max(signal) + 1));
    
    for i = 1 : length(psk31)
       fprintf(file, '0x%X\n', psk31(i)); 
    end
    
    fclose(file);
end