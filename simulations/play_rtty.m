function play_rtty(message, sn)
    signal = awgn(gen_rtty(8000, 1445, 1275, 1 / 45.45, 2, message), sn);
    signal = signal / max(signal);

    soundsc(signal, 8000);
end