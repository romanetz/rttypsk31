function play_psk31(message, sn)
    signal = awgn(gen_psk(8000, 1300, message), sn);
    
    soundsc(signal, 8000);
end