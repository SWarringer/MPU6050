Skriv / ta emot:
i2c_master_transmit() - Skriv data till register address
i2c_master_receive() - Läs från register address
i2c_master_transmit_receive() - Skriv och läs sedan

Skapa först bussen, sedan lägg till enheter till bussen (Lite som att skapa can interface sedan öppna kanalen?)

Reading a register ALWAYS requires a WRITE first!

    You write the register address you want to read from

    The device's internal pointer moves to that register

    You then read the value

It's like asking a librarian:

    "I want the book at shelf 0x75" (write the address)

    Librarian fetches it (internal pointer moves)

    "Here's the book" (you read the value)
