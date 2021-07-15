import re
import tkinter as tk
from tkinter import ttk
from typing import final
from serial import Serial
from serial.tools import list_ports


class App(tk.Frame):
    def __init__(self, master):
        super().__init__(master)
        self.pack()

        master.title('Winder controls')
        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=1)
        self.columnconfigure(2, weight=1)

        self.serial = Serial(baudrate=115200)
        self.inputs = []
        self.ports = self.get_port_list()
        self.is_connected = False

        label = ttk.Label(self, text="Select port")
        label.grid(row=0, columnspan=3)
        self.portSelector = ttk.Combobox(self, values=self.ports)
        self.portSelector.grid(row=1, column=0, sticky=tk.W, padx=5)

        self.connectButton = tk.Button(
            self, command=self.toggle_connection, text="Connect")
        self.connectButton.grid(row=1, column=1, sticky=tk.E, padx=5)

        self.refreshButton = tk.Button(
            self, command=self.update_port_list, text="Refresh")
        self.refreshButton.grid(row=1, column=2, sticky=tk.E, padx=5)

        for param in ['Begin winds', 'End winds', 'Length wind', 'Step wind']:
            self.create_input(param)

    def get_port_list(self):
        return list(map(lambda port: port.name, list_ports.comports()))

    def update_port_list(self):
        self.disconnect_from_port()
        self.ports = self.get_port_list()
        self.portSelector.set('')
        self.portSelector['values'] = self.ports

    def toggle_connection(self):
        if self.is_connected:
            self.disconnect_from_port()
        else:
            self.connect_to_port()

    def connect_to_port(self):
        port = self.portSelector.get()
        self.serial.port = port
        try:
            self.serial.open()
            for input in self.inputs:
                input['state'] = tk.NORMAL
            self.connectButton.configure(text="Disconnect")
            self.startButton['state'] = tk.NORMAL
            self.is_connected = True
        except:
            pass

    def disconnect_from_port(self):
        self.serial.close()
        self.is_connected = False
        self.connectButton.configure(text="Connect")
        for input in self.inputs:
            input['state'] = tk.DISABLED
        self.startButton['state'] = tk.DISABLED

    def create_input(self, paramName):
        variable = tk.StringVar(self, '0')
        send_to_serial = self.create_serial_sender(paramName[:1], variable)
        label = tk.Label(self, text=paramName)
        label.grid(sticky=tk.W)
        input = tk.Spinbox(self, from_=0, to=100, width=42,
                           textvariable=variable, command=send_to_serial)
        input.grid(columnspan=3)
        input.bind('<Return>', send_to_serial)
        input['state'] = tk.DISABLED
        self.inputs.append(input)

    def create_serial_sender(self, commandID, variable):
        return lambda *args: self.serial.write(bytes(commandID + variable.get() + '\n', 'ascii'))

    def destroy(self):
        self.disconnect_from_port()
        return super().destroy()


root = tk.Tk()
myapp = App(root)
myapp.mainloop()
