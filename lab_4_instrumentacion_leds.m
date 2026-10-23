clc;
clear;
close all;

dispositivo = "Dev2";

dq = daq("ni");

% Entrada del LM358
addinput(dq, dispositivo, "ai0", "Voltage");

% Salidas digitales para los LED
addoutput(dq, dispositivo, "Port0/Line0:2", "Digital");

% Inicializar LED apagados
write(dq, [0 0 0]);

% Grafica
figure;
h = animatedline('Color','r','LineWidth',1.5);
xlabel('Tiempo (s)');
ylabel('Temperatura (°C)');
title('Temperatura medida con LM35');
grid on;

tic;

try
    while ishandle(gcf)

        datos = read(dq);

        voltaje = datos{1,1};

        % Conversion a grados celsius
        temperatura = voltaje * 100;

        % Clasificacion y control de LED
        if temperatura >= 37.2 && temperatura <= 37.8

            write(dq, [1 0 0]);
            estado = "VERDE";

        elseif (temperatura >= 36.5 && temperatura < 37.2) || ...
               (temperatura > 37.8 && temperatura <= 38.5)

            write(dq, [0 1 0]);
            estado = "AMARILLO";

        else

            write(dq, [0 0 1]);
            estado = "ROJO";

        end

        tiempo = toc;
        addpoints(h, tiempo, temperatura);
        drawnow limitrate;

        fprintf('Voltaje: %.4f V | Temperatura: %.2f °C | LED: %s\n', ...
            voltaje, temperatura, estado);

        pause(1);
    end

catch ME
    fprintf('Error: %s\n', ME.message);
end

% Intentar apagar los LED al finalizar
try
    write(dq, [0 0 0]);
catch
end

disp('Programa finalizado.');
