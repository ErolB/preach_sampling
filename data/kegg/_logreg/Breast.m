Obs = [3.8823529411764706 -0.2823879856209541 -0.0
1.6666666666666667 -0.18496147240232644 -0.0
1.6666666666666667 0.6746806899741365 -0.0
2.275 0.5415273086984738 -0.0
1.72 0.4278716753216763 -0.0
1.02 0.8958294247917608 -0.0
0.505 -0.29714448458825604 1.1288475184363773
0.6733333333333333 0.013044869849237617 1.1102230246251565e-16
3.6 0.5121084750337556 -0.0
0.6733333333333333 -0.3864510701782884 -0.0
0.505 -0.29714448458825604 -0.0
0.0 -0.51935961951033 -0.0
0.0 0.4218989858866909 -0.0
0.0 0.8854368580895726 1.1102230246251565e-16
2.0 0.3517315544017827 -2.2204460492503128e-16
0.0 -0.1262272832958479 2.811082427000445
0.0 0.08332738830444561 -0.0
0.0 0.19950324778746262 -0.0
0.0 -0.367807772565728 3.258096538021482];
Cat = [1 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2]';
B = mnrfit(Obs, Cat)
fid = fopen('Breast.ge.param', 'w');
fprintf(fid, '%f\n', B);
fclose(fid);
