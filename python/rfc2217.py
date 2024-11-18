import asyncio
from serial_asyncio import open_serial_connection

# Configuración del servidor
RFC2217_PORT = 4000
SERIAL_PORT = 'COM3'  # Cambia esto al puerto que desees usar
BAUDRATE = 115200

async def handle_client(reader, writer, serial_writer, serial_reader):
    try:
        print("Cliente conectado.")
        while True:
            # Leer datos del cliente y enviarlos al puerto serie
            data = await reader.read(1024)
            if not data:
                break
            serial_writer.write(data)
            await serial_writer.drain()
            print(f"Enviado al puerto serie: {data.decode().strip()}")

            # Leer respuesta del puerto serie y enviarla al cliente
            serial_data = await serial_reader.read(1024)
            if serial_data:
                writer.write(serial_data)
                await writer.drain()
                print(f"Enviado al cliente: {serial_data.decode().strip()}")
    except Exception as e:
        print(f"Error en la conexión: {e}")
    finally:
        print("Cliente desconectado.")
        writer.close()
        await writer.wait_closed()

async def start_rfc2217_server():
    # Abre la conexión al puerto serie
    print(f"Conectando al puerto serie {SERIAL_PORT} a {BAUDRATE} baudios...")
    serial_reader, serial_writer = await open_serial_connection(url=SERIAL_PORT, baudrate=BAUDRATE)

    # Inicia el servidor TCP
    server = await asyncio.start_server(
        lambda r, w: handle_client(r, w, serial_writer, serial_reader),
        '0.0.0.0',
        RFC2217_PORT
    )

    async with server:
        print(f"Servidor RFC2217 corriendo en 0.0.0.0:{RFC2217_PORT}")
        await server.serve_forever()

try:
    asyncio.run(start_rfc2217_server())
except KeyboardInterrupt:
    print("Servidor detenido manualmente.")
