RTTY & PSK31 decoding software

Theory of operation:
The PSK and RTTY decoders can be decoded using a simular logical structure. Signals will be sampled using an 8000hz ADC. The signal will be fed through a time delay tan lock loop (TDTL). The error signal from this loop can be used to detect the RTTY and PSK31 symbols from the audio.

These symbols can be decoded through use of a runloop and an auxiliary timer. The output will be printed on a standard character display.

File structure:
Everything in the root directory is the source for the actual Microchip hardware implementation. It is possible to simulate this in Microchip's MPLAB development toolkit, but the code will not work in a regular PC operating system without modification.

How to use the MATLAB:
There are three particularly valuable MATLAB scripts worth understanding. The first and second are src/tdtl.m and src/tdtl_psk.m. These are two very similar MATLAB scripts. The first plots the error signal received from an oscillating FSK signal. The second does the same for a PSK modulator. This shows how the TDTL will work for decoding the signals.

The third script is src/gen_rtty_for_mplab_sim.m. This is a function definition that takes who parameters: a message and a filename. This is used to generate test signals for the MPLAB simulations. Currently the message must be the character representation of a binary stream.
    For instance: gen_rtty_for_mplab_sim('abcd', 'myfile.txt') fills the file 'myfile.txt' with the audio samples that represent the message 'abcd' using standard RTTY symbols, frequencies, and symbol rates.

How to run the MPLAB simulations:
1. Install MPLAB from the Microchip website
2. Install the Microchip dsPIC33 C compiler from the Microchip website
3. Open DSP.mcp (the Microchip project) in MPLAB
4. Select the simulator for debugging
5. Run

How to run the Real Thing (you'll find it rather unexciting):
1. Follow steps 1-3 above for running the MPLAB simulations
2. Select ICD2 for debugging
3. Run
