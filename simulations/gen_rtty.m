function output = gen_rtty(Fs, Fspace, Fmark, Tsymbol, stopBits, message)
    RL = containers.Map('KeyType', 'char');
    RF = containers.Map('KeyType', 'char');
    
    RL('NULL') = '00000';
    RL('E') = '00001';
    RL(sprintf('\n')) = '00010'; %LF
    RL('A') = '00011';
    RL(' ') = '00100'; %sp
    RL('S') = '00101';
    RL('I') = '00110';
    RL('U') = '00111';
    RL('CR') = '01000';
    RL('D') = '01001';
    RL('R') = '01010';
    RL('J') = '01011';
    RL('N') = '01100';
    RL('F') = '01101';
    RL('C') = '01110';
    RL('K') = '01111';
    RL('T') = '10000';
    RL('Z') = '10001';
    RL('L') = '10010';
    RL('W') = '10011';
    RL('H') = '10100';
    RL('Y') = '10101';
    RL('P') = '10110';
    RL('Q') = '10111';
    RL('O') = '11000';
    RL('B') = '11001';
    RL('G') = '11010';
    RL('FIGS') = '11011';
    RL('M') = '11100';
    RL('X') = '11101';
    RL('V') = '11110';
    RL('LTRS') = '11111';

    RF('NULL') = '00000';
    RF('3') = '00001';
    RF(sprintf('\n')) = '00010'; %LF
    RF('-') = '00011';
    RF(' ') = '00100'; %SP
    RF('''') = '00101';
    RF('8') = '00110';
    RF('7') = '00111';
    RF('CR') = '01000';
    RF('ENQ') = '01001';
    RF('4') = '01010';
    RF('BEL') = '01011';
    RF(',') = '01100';
    RF('!') = '01101';
    RF(':') = '01110';
    RF('(') = '01111';
    RF('5') = '10000';
    RF('"') = '10001';
    RF(')') = '10010';
    RF('2') = '10011';
    RF('#') = '10100';
    RF('6') = '10101';
    RF('0') = '10110';
    RF('1') = '10111';
    RF('9') = '11000';
    RF('?') = '11001';
    RF('&') = '11010';
    RF('FIGS') = '11011';
    RF('.') = '11100';
    RF('/') = '11101';
    RF(';') = '11110';
    RF('LTRS') = '11111';
    
    addErrorBits = @(x) ['0' fliplr(x) '2'];
    
    bitmessage = [];
    decodeState = 'letters';
    for i = 1 : length(message)
        character = upper(message(i));
        
        if(isKey(RL, character))
            if(~strcmp(decodeState, 'letters'))
                decodeState = 'letters';
                bitmessage = [bitmessage addErrorBits(RF('LTRS'))];
            end
            
            bitmessage = [bitmessage addErrorBits(RL(character))];
        elseif(isKey(RF, character))
            if(~strcmp(decodeState, 'figures'))
                decodeState = 'figures';
                bitmessage = [bitmessage addErrorBits(RL('FIGS'))];
            end
            
            bitmessage = [bitmessage addErrorBits(RF(character))];
        else
            bitmessage = [bitmessage addErrorBits(RL('NULL'))];
        end
    end
    
    Ts = 1 / Fs;
    
    t = 0;
    
    output = [];
    Tsymbolc = Tsymbol;
    for i = 1 : length(bitmessage)
        if(bitmessage(i) == '1')
            Fsymbol = Fmark;
            Tsymbolc = Tsymbol;
        elseif(bitmessage(i) == '0')
            Fsymbol = Fspace;
            Tsymbolc = Tsymbol;
        elseif(bitmessage(i) == '2')
            Fsymbol = Fmark;
            Tsymbolc = Tsymbol * stopBits;
        end
        
        while t < i * Tsymbolc
            output = [output sin(2 * pi() * Fsymbol * t)];
            t = t + Ts;
        end
    end
end
