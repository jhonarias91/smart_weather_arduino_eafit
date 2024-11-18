package com.jhonarias91;

// Importamos las librerías necesarias para la comunicación serie y WebSocket
import com.fazecast.jSerialComm.SerialPort;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import org.json.JSONObject;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * Este programa actúa como un puente entre un puerto serie y una conexión WebSocket.
 * Lee datos desde un puerto serie (por ejemplo, conectado a un microcontrolador) y los envía a un servidor WebSocket.
 * También recibe datos del servidor WebSocket y los envía al puerto serie.
 */
public class SerialWebSocketBridge {

    // Identificador único para este cliente (puede ser cualquier valor único)
    private static final String ID = "mi_id";

    // URL del servidor WebSocket (se añade el ID al final de la URL base)
    private static final String WSS_URL = "wss://ws.davinsony.com/" + ID;

    // Nombre del puerto serie (cambia esto al puerto serie correcto)
    private static final String SERIAL_PORT = "COM5"; // Por ejemplo, "COM5" en Windows o "/dev/ttyUSB0" en Linux

    // Velocidad de comunicación para el puerto serie (baudios)
    private static final int BAUD_RATE = 9600;

    // Variables para el puerto serie y el cliente WebSocket
    private SerialPort serialPort;
    private WebSocketClient webSocketClient;

    // Constructor de la clase
    public SerialWebSocketBridge() {

        // Inicializamos el puerto serie
        serialPort = SerialPort.getCommPort(SERIAL_PORT); // Obtenemos el puerto serie por nombre
        serialPort.setBaudRate(BAUD_RATE); // Configuramos la velocidad de comunicación
        serialPort.openPort(); // Abrimos el puerto para comunicación

        // Configuramos el cliente WebSocket
        try {
            // Creamos una nueva instancia de WebSocketClient, conectándonos a la URL especificada
            webSocketClient = new WebSocketClient(new URI(WSS_URL)) {

                // Método llamado cuando se abre la conexión WebSocket
                @Override
                public void onOpen(ServerHandshake handshakedata) {
                    System.out.println("Conexión WebSocket abierta: " + WSS_URL);
                    // Iniciamos un nuevo hilo para leer datos desde el puerto serie
                    new Thread(() -> readFromSerial()).start();
                }

                // Método llamado cuando se recibe un mensaje desde el servidor WebSocket
                @Override
                public void onMessage(String message) {
                    // Parseamos el mensaje recibido como JSON
                    JSONObject data = new JSONObject(message);
                    System.out.println("Mensaje recibido: " + data.toString());

                    // Extraemos el campo "msg" del JSON
                    String msg = data.optString("msg");
                    if (msg != null) {
                        // Enviamos el mensaje al puerto serie, añadiendo un salto de línea
                        sendToSerial(msg + "\n");
                    }
                }

                // Método llamado cuando se cierra la conexión WebSocket
                @Override
                public void onClose(int code, String reason, boolean remote) {
                    System.out.println("Conexión WebSocket cerrada: " + reason);
                }

                // Método llamado cuando ocurre un error en la conexión WebSocket
                @Override
                public void onError(Exception ex) {
                    System.err.println("Error en WebSocket: " + ex.getMessage());
                }
            };

            // Conectamos al servidor WebSocket
            webSocketClient.connect();
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }
    }

    /**
     * Lee continuamente datos desde el puerto serie.
     * Cualquier dato leído se envía al servidor WebSocket, a menos que contenga la palabra "Recibido".
     */
    private void readFromSerial() {
        while (serialPort.isOpen()) {
            byte[] buffer = new byte[1024]; // Buffer para almacenar datos entrantes
            int bytesRead = serialPort.readBytes(buffer, buffer.length); // Leemos bytes desde el puerto serie
            if (bytesRead > 0) {
                // Convertimos los bytes leídos en una cadena
                String line = new String(buffer, 0, bytesRead).trim();

                // Si la línea no contiene "Recibido", la enviamos al servidor WebSocket
                if (!line.contains("Recibido")) {
                    sendToWebSocket(line);
                }
            }
        }
    }

    /**
     * Envía un mensaje al puerto serie.
     * @param message El mensaje a enviar.
     */
    private void sendToSerial(String message) {
        if (serialPort.isOpen()) {
            // Escribimos el mensaje en el puerto serie
            serialPort.writeBytes(message.getBytes(), message.length());
        }
    }

    /**
     * Envía un mensaje al servidor WebSocket.
     * @param message El mensaje a enviar.
     */
    private void sendToWebSocket(String message) {
        if (webSocketClient != null && webSocketClient.isOpen()) {
            // Creamos un objeto JSON para el mensaje
            JSONObject json = new JSONObject();
            json.put("msg", message); // Añadimos el campo "msg" con el mensaje
            json.put("to", ID); // Añadimos el campo "to" con el ID
            json.put("from", ID); // Añadimos el campo "from" con el ID

            // Enviamos el objeto JSON como cadena al servidor WebSocket
            webSocketClient.send(json.toString());
        }
    }

    // Método principal para iniciar el programa
    public static void main(String[] args) {
        new SerialWebSocketBridge(); // Creamos una nueva instancia del puente
    }
}
