clear;clc;
T = readtable("pid_datos");
B = readmatrix("pid_datos");
plot(B);
To=22;
yline(290,'-.r','T maxima');
yline(To,'-.r','T ambiente');
title("lectura de NTC");
xlabel('t_{segundos}')
ylabel('Temperatura_{°C}')
K = 290-22; % ganancia
rmax=(K*0.632)+22;
yline(rmax,'-.r','63 del maximo');
xline(300,'-b');
xline(953,'-b');
tau=953-287;
s = tf('s');
P = (K)/(tau*s+1); % función de transferencia del modelo
[y,t] = step(P,1653);  % model step response
figure
plot(t+300,y+To);
hold
plot(B,'r:');
title("aproximacion de la curva");
xlabel('t_{segundos}')
ylabel('Temperatura_{°C}')
legend('aproximacion','lectura NTC')
%__________________________________
figure
% este PID es que que teniamos por defecto con el ejemplo de PID que
% descarge para aeduino
Kp = 9.1;
Ki = 0.3;
Kd = 1.8;
C = pid(Kp,Ki,Kd);
T = feedback(C*P,1);
t = 0:0.01:10;
step(T,t)