function h = dec2hexs(d)
    if(d >= 0)
        h = dec2hex(abs(d));
    else
        h = dec2hex(bitand(bitcmp(abs(d), 16) + 1, hex2dec('FFFF')));
    end
end
