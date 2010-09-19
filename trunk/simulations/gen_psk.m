function output = gen_psk(Fs, F0, deltap, Tsymbol, message)
    bitmessage = message;
    
    Ts = 1 / Fs;
    
    T = length(bitmessage) * Tsymbol;
    
    t = 0;
    
    output = [];
    for i = 1:length(bitmessage)
        if(bitmessage(i) == '1')
            m = 1.0;
        else
            m = 0.0;
        end
        
        while t < i * Tsymbol
            output = [output sin(2 * pi() * F0 * t + m * deltap)];
            t = t + Ts;
        end
    end
end
