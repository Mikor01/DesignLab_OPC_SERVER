import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import sys
import threading
from opcua import Client, ua
from datetime import datetime
import time
import socket
import os

NODE_COMMAND_1 = "ns=1;s=IN1_control"
NODE_COMMAND_2 = "ns=1;s=IN2_control"
NODE_STATUS = "ns=1;s=uart_status"

def log_to_file(message):
    current_datetime = datetime.now()
    filename = current_datetime.strftime("%Y-%m-%d") + "_logs.txt"
    timestamp = current_datetime.strftime("%Y-%m-%d %H:%M:%S")
    try:
        with open(filename, "a", encoding="utf-8") as f:
            f.write(f"{timestamp}; {message}\n")
    except Exception as e:
        print(f"Error while writing log: {e}")

def connect_with_retry(client, url, retries=10):
    for attempt in range(1, retries + 1):
        try:
            print(f"Connecting to OPC UA server (attempt {attempt})...")
            client.connect()
            print(f"Connected with OPC UA: {url}")
            log_to_file(f"Connected with OPC UA {url}")
            return True
        except (OSError, socket.error, ConnectionError) as e:
            log_to_file(f"Connection failed (Attempt {attempt})")
            print(f"Connection failed: {e}. Retrying...")
            time.sleep(20) 
    return False

def read_server_status(client):
    try:
        node = client.get_node(NODE_STATUS)
        val = node.get_value()
        print(f"[SERVER STATUS]: {val}")
    except Exception as e:
        print(f"Error while trying to read status: {e}")

def switch_inputs(client, node_command_id, out_choose):
    try:
        node_command = client.get_node(node_command_id)
        current_val = node_command.get_value()
        
        if current_val == out_choose:
            print(f"Value {out_choose} is already set on {node_command_id}. Skipping write.")
            return True

        node_command.set_value(ua.Variant(out_choose, ua.VariantType.Int32))
        read_server_status(client)
        return True
    except Exception as e:
        print(f"Error in switch_inputs: {e}")
        return False

def string_commands(client, command):
    try:
        node_status = client.get_node(NODE_STATUS)
        node_status.set_value(ua.Variant(command, ua.VariantType.String))
        read_server_status(client)
        return True
    except Exception as e:
        print(f"Error in string_commands: {e}")
        return False

class Printlog:
    def __init__(self, text_widget):
        self.text_widget = text_widget

    def write(self, str_val):
        if str_val.strip():
            timestamp = datetime.now().strftime("[%H:%M:%S] ")
            str_val = f"{timestamp}{str_val}"
            if not str_val.endswith('\n'):
                str_val += '\n'
        
        if str_val == '\n':
            return

        self.text_widget.configure(state='normal')
        self.text_widget.insert('end', str_val)
        self.text_widget.see('end')
        self.text_widget.configure(state='disabled')

    def flush(self):
        pass

class Gui:
    def __init__(self, root):
        self.root = root
        self.root.title("OPC UA Control Panel")
        self.root.geometry("650x580")
        
        self.client = None
        self.connected = False

        self.create_widgets()
        
        sys.stdout = Printlog(self.log_area)
        self.load_existing_logs()

    def load_existing_logs(self):
        filename = datetime.now().strftime("%Y-%m-%d") + "_logs.txt"
        if os.path.exists(filename):
            try:
                with open(filename, "r", encoding="utf-8") as f:
                    content = f.read()
                    self.log_area.configure(state='normal')
                    self.log_area.insert('end', content)
                    self.log_area.insert('end', "-"*30 + " NEW SESSION " + "-"*30 + "\n")
                    self.log_area.see('end')
                    self.log_area.configure(state='disabled')
            except Exception as e:
                print(f"Could not load existing logs: {e}")

    def create_widgets(self):
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        config_frame = ttk.LabelFrame(main_frame, text="Server Configuration")
        config_frame.pack(fill=tk.X, pady=5)

        ttk.Label(config_frame, text="IP:").grid(row=0, column=0, padx=5, pady=5)
        self.ip_entry = ttk.Entry(config_frame)
        self.ip_entry.insert(0, "192.168.1.1")
        self.ip_entry.grid(row=0, column=1, padx=5, pady=5, sticky="ew")

        ttk.Label(config_frame, text="Port:").grid(row=0, column=2, padx=5, pady=5)
        self.port_entry = ttk.Entry(config_frame, width=8)
        self.port_entry.insert(0, "4840")
        self.port_entry.grid(row=0, column=3, padx=5, pady=5)

        conn_frame = ttk.Frame(main_frame)
        conn_frame.pack(fill=tk.X, pady=5)
        
        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.start_connection)
        self.btn_connect.pack(side=tk.LEFT, padx=5, pady=5, expand=True, fill=tk.X)
        
        self.btn_disconnect = ttk.Button(conn_frame, text="Disconnect", command=self.disconnect_client, state=tk.DISABLED)
        self.btn_disconnect.pack(side=tk.LEFT, padx=5, pady=5, expand=True, fill=tk.X)

        control_frame = ttk.LabelFrame(main_frame, text="Input/Output Control")
        control_frame.pack(fill=tk.X, pady=5)

        ttk.Label(control_frame, text="Input:").grid(row=0, column=0, padx=5, pady=5)
        self.in_var = tk.IntVar(value=1)
        self.in_1_rb = ttk.Radiobutton(control_frame, text="IN 1", variable=self.in_var, value=1)
        self.in_1_rb.grid(row=0, column=1)
        self.in_2_rb = ttk.Radiobutton(control_frame, text="IN 2", variable=self.in_var, value=2)
        self.in_2_rb.grid(row=0, column=2)

        ttk.Label(control_frame, text="Output (1-16):").grid(row=1, column=0, padx=5, pady=5)
        self.out_var = tk.IntVar(value=1)
        self.spin_out = ttk.Spinbox(control_frame, from_=1, to=16, textvariable=self.out_var, width=5)
        self.spin_out.grid(row=1, column=1, sticky="w")

        ttk.Button(control_frame, text="SET Output", command=self.action_set).grid(row=2, column=0, columnspan=2, pady=10, padx=5, sticky="ew")
        ttk.Button(control_frame, text="Turn OFF", command=self.action_off).grid(row=2, column=2, pady=10, padx=5, sticky="ew")

        status_display_frame = ttk.Frame(control_frame)
        status_display_frame.grid(row=3, column=0, columnspan=3, pady=5, sticky="ew")
        
        ttk.Label(status_display_frame, text="Status:").pack(side=tk.LEFT, padx=5)
        self.lbl_status_in1 = ttk.Label(status_display_frame, text="IN1: --", foreground="blue")
        self.lbl_status_in1.pack(side=tk.LEFT, padx=10)
        self.lbl_status_in2 = ttk.Label(status_display_frame, text="IN2: --", foreground="blue")
        self.lbl_status_in2.pack(side=tk.LEFT, padx=10)
        
        log_frame = ttk.LabelFrame(main_frame, text="System Logs")
        log_frame.pack(fill=tk.BOTH, expand=True, pady=5)

        self.log_area = scrolledtext.ScrolledText(log_frame, state='disabled', height=12, font=("Consolas", 9))
        self.log_area.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        footer_label = ttk.Label(self.root, text="OPC_client by RRS", anchor=tk.E, relief=tk.SUNKEN)
        footer_label.pack(side=tk.BOTTOM, fill=tk.X)

    def start_connection(self):
        ip = self.ip_entry.get()
        port = self.port_entry.get()
        url = f"opc.tcp://{ip}:{port}"
        self.btn_connect.config(state=tk.DISABLED)
        self.client = Client(url)
        t = threading.Thread(target=self.connect_thread, args=(url,))
        t.daemon = True
        t.start()

    def connect_thread(self, url):
        if connect_with_retry(self.client, url):
            self.connected = True
            self.root.after(0, lambda: self.btn_disconnect.config(state=tk.NORMAL))
            self.root.after(0, self.refresh_io_status)
        else:
            self.root.after(0, lambda: self.btn_connect.config(state=tk.NORMAL))

    def disconnect_client(self):
        try:
            if self.client: self.client.disconnect()
            print("Disconnected from server.")
        except: pass
        self.connected = False
        self.btn_connect.config(state=tk.NORMAL)
        self.btn_disconnect.config(state=tk.DISABLED)
        self.lbl_status_in1.config(text="IN1: --")
        self.lbl_status_in2.config(text="IN2: --")

    def refresh_io_status(self):
        if not self.connected: return
        def _read():
            try:
                val1 = self.client.get_node(NODE_COMMAND_1).get_value()
                val2 = self.client.get_node(NODE_COMMAND_2).get_value()
                def _update_ui():
                    self.lbl_status_in1.config(text=f"IN1: {val1}")
                    self.lbl_status_in2.config(text=f"IN2: {val2}")
                    self.root.after(2000, self.refresh_io_status)
                self.root.after(0, _update_ui)
            except Exception as e: print(f"Error reading status: {e}")
        t = threading.Thread(target=_read)
        t.daemon = True
        t.start()

    def action_set(self):
        if not self.connected:
            print("Action failed: Not connected!")
            return
        in_val = self.in_var.get()
        out_val = self.out_var.get()
        try:
            val1 = self.client.get_node(NODE_COMMAND_1).get_value()
            val2 = self.client.get_node(NODE_COMMAND_2).get_value()
            if in_val == 1 and out_val == val2:
                print(f"Conflict: Output {out_val} is already assigned to IN 2!")
                return
            if in_val == 2 and out_val == val1:
                print(f"Conflict: Output {out_val} is already assigned to IN 1!")
                return
            node_id = NODE_COMMAND_1 if in_val == 1 else NODE_COMMAND_2
            print(f"Executing: SET IN {in_val} -> OUT {out_val}")
            if switch_inputs(self.client, node_id, out_val):
                self.refresh_io_status()
        except Exception as e: print(f"GUI Error: {e}")

    def action_off(self):
        if not self.connected: return
        in_val = self.in_var.get()
        node_id = NODE_COMMAND_1 if in_val == 1 else NODE_COMMAND_2
        print(f"Executing: TURN OFF IN {in_val}")
        if switch_inputs(self.client, node_id, -1):
            self.refresh_io_status()

    def send_cmd(self, cmd):
        if not self.connected: return
        print(f"Command Sent: {cmd}")
        string_commands(self.client, cmd)

if __name__ == "__main__":
    root = tk.Tk()
    app = Gui(root)
    try:
        root.mainloop()
    finally:
        try:
            if app.client: app.client.disconnect()
        except: pass