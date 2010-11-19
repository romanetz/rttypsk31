function output = gen_psk(Fs, F0, message)
    P = containers.Map('KeyType', 'char');
    
    P(char(0)) = '1010101011';
    P(char(1)) = '1011011011';
    P(char(2)) = '1011101101';
    P(char(3)) = '1101110111';
    P(char(4)) = '1011101011';
    P(char(5)) = '1101011111';
    P(char(6)) = '1011101111';
    P(char(7)) = '1011111101';
    P(char(8)) = '1011111111';
    P(char(9)) = '11101111';
    P(char(10)) = '11101'; %l/feed
    P(char(11)) = '1101101111';
    P(char(12)) = '1011011101';
    P(char(13)) = '11111'; %c/ret
    P(char(14)) = '1101110101';
    P(char(15)) = '1110101011';
    P(char(16)) = '1011110111';
    P(char(17)) = '1011110101';
    P(char(18)) = '1110101101';
    P(char(19)) = '1110101111';
    P(char(20)) = '1101011011';
    P(char(21)) = '1101101011';
    P(char(22)) = '1101101101';
    P(char(23)) = '1101010111';
    P(char(24)) = '1101111011';
    P(char(25)) = '1101111101';
    P(char(26)) = '1110110111';
    P(char(27)) = '1101010101';
    P(char(28)) = '1101011101';
    P(char(29)) = '1110111011';
    P(char(30)) = '1011111011';
    P(char(31)) = '1101111111';
    P(' ') = '1';
    P('!') = '111111111';
    P('"') = '101011111';
    P('#') = '111110101';
    P('$') = '111011011';
    P('%') = '1011010101';
    P('&') = '1010111011';
    P('''') = '101111111';
    P('(') = '11111011';
    P(')') = '11110111';
    P('*') = '101101111';
    P('+') = '111011111';
    P(',') = '1110101';
    P('-') = '110101';
    P('.') = '1010111';
    P('/') = '110101111';
    P('0') = '10110111';
    P('1') = '10111101';
    P('2') = '11101101';
    P('3') = '11111111';
    P('4') = '101110111';
    P('5') = '101011011';
    P('6') = '101101011';
    P('7') = '110101101';
    P('8') = '110101011';
    P('9') = '110110111';
    P(':') = '11110101';
    P(';') = '110111101';
    P('<') = '111101101';
    P('=') = '1010101';
    P('>') = '111010111';
    P('?') = '1010101111';
    P('@') = '1010111101';
    P('A') = '1111101';
    P('B') = '11101011';
    P('C') = '10101101';
    P('D') = '10110101';
    P('E') = '1110111';
    P('F') = '11011011';
    P('G') = '11111101';
    P('H') = '101010101';
    P('I') = '1111111';
    P('J') = '111111101';
    P('K') = '101111101';
    P('L') = '11010111';
    P('M') = '10111011';
    P('N') = '11011101';
    P('O') = '10101011';
    P('P') = '11010101';
    P('Q') = '111011101';
    P('R') = '10101111';
    P('S') = '1101111';
    P('T') = '1101101';
    P('U') = '101010111';
    P('V') = '110110101';
    P('W') = '101011101';
    P('X') = '101110101';
    P('Y') = '101111011';
    P('Z') = '1010101101';
    P('[') = '111110111';
    P('\') = '111101111';
    P(']') = '111111011';
    P('^') = '1010111111';
    P('_') = '101101101';
    P('`') = '1011011111';
    P('a') = '1011';
    P('b') = '1011111';
    P('c') = '101111';
    P('d') = '101101';
    P('e') = '11';
    P('f') = '111101';
    P('g') = '1011011';
    P('h') = '101011';
    P('i') = '1101';
    P('j') = '111101011';
    P('k') = '10111111';
    P('l') = '11011';
    P('m') = '111011';
    P('n') = '1111';
    P('o') = '111';
    P('p') = '111111';
    P('q') = '110111111';
    P('r') = '10101';
    P('s') = '10111';
    P('t') = '101';
    P('u') = '110111';
    P('v') = '1111011';
    P('w') = '1101011';
    P('x') = '11011111';
    P('y') = '1011101';
    P('z') = '111010101';
    P('{') = '1010110111';
    P('|') = '110111011';
    P('}') = '1010110101';
    P('~') = '1011010111';
    P(char(127)) = '1110110101';
    
    
    bitmessage = '0000';
    for i = 1 : length(message)
        character = message(i);
        
        if(isKey(P, character))
            bitmessage = [bitmessage P(character) '00'];
        else
            bitmessage = [bitmessage '00'];
        end
    end
    
    Tsymbol = 1 / 31.25;
    Ts = 1 / Fs;
    t = 0;
    
    output = [];
    tm = 0;
    for i = 1:length(bitmessage)
        if(bitmessage(i) == '1')
            m = 0.0;
        else
            m = 1.0;
        end
        
        tb = t;
        
        while t < i * Tsymbol
            output = [output sin(2 * pi() * F0 * t) * cos(pi() * tm / Tsymbol)];
            t = t + Ts;
            tm = tm + m * Ts;
        end
    end
end
