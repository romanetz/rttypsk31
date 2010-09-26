function gen_rtty_for_mplab_sim(message, filename)
    file = fopen(filename, 'w');

    scale = 2^11 - 1;
    
    rtty = round(scale * (gen_rtty(8000, 1275, 1445, 1 / 70, message) + 1));
    
    for i = 1 : length(rtty)
       fprintf(file, '0x%X\n', rtty(i)); 
    end
    
    fclose(file);
end