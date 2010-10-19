function gen_psk31_for_mplab_sim(message, filename)
    file = fopen(filename, 'w');

    scale = 2^11 - 1;
    
    psk31 = round(scale * (gen_psk(8000, 1350, message) + 1));
    
    for i = 1 : length(psk31)
       fprintf(file, '0x%X\n', psk31(i)); 
    end
    
    fclose(file);
end