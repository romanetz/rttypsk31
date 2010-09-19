function output = gen_rtty(Fs, Fspace, Fmark, Tsymbol, message)
    bitmessage = message;
    
    Ts = 1 / Fs;
    
    t = 0;
    
    output = [];
    for i = 1:length(bitmessage)
        if(bitmessage(i) == '1')
            Fsymbol = Fmark;
        else
            Fsymbol = Fspace;
        end
        
        while t < i * Tsymbol
            output = [output sin(2 * pi() * Fsymbol * t + pi() / 8)];
            t = t + Ts;
        end
    end
end
