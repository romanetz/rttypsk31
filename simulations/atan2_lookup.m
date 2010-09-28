function ang = atan2_lookup(x, y)
    %x = floor(x / (2.0 ^ 8));
    %y = floor(y / (2.0 ^ 8));

    ang = atan2(single(x), single(y)); 
end