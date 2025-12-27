from opcua import Client, ua
from datetime import datetime
import time
import socket

#server's data
SERVER_URL = "opc.tcp://<IP>" #type your's server's IP
NODE_COMMAND_1 = "ns=1;s=IN1_control"
NODE_COMMAND_2 = "ns=1;s=IN2_control"
NODE_STATUS = "ns=1;s=uart_status"

#save logs to file
def log_to_file(message):
    current_datetime = datetime.now()
    filename = current_datetime.strftime("%Y-%m-%d") + "_logs.txt"
    timestamp = current_datetime.strftime("%Y-%m-%d %H:%M:%S")
    try:
        with open(filename, "a") as f:
            f.write(f"{timestamp}; {message}\n")
    except Exception as e:
        print(f"Error while writing log: {e}")

#connecting to serber with retries
def connect_with_retry(client, retries=10): #set number of retries
    for attempt in range(1, retries + 1):
        try:
            print(f"Connecting to OPC UA server (attempt {attempt})...")
            client.connect()
            print("Connected with OPC UA:", SERVER_URL)
            message = "Connected with OPC UA " + SERVER_URL
            log_to_file(message)
            return True
        except (OSError, socket.error, ConnectionError) as e:
            log_to_file(f"Connection failed (Attempt {attempt})")
            print(f"Connection failed: {e}. Retrying...")
            time.sleep(20) #set time beetven retries
    return False

#user's manual
def print_help():
    print("\n=========== Available Commands ===========")
    print("in <1-2> out <1-16>  - Set input to output")
    print("in <1-2> off          - Turn off input")
    print("release               - Release controller")
    print("clear                 - Clear display")
    print("draw                  - Redraw display")
    print("screen                - Enable screensaver")
    print("status                - Get current status")
    print("\nExamples:")
    print("  in 1 out 5          - Set IN1 to OUT5")
    print("  in 2 out 12         - Set IN2 to OUT12")
    print("  in 1 off            - Turn off IN1")
    print("==========================================")

#get input/output status
def read_server_status(client):
    try:
        node = client.get_node(NODE_STATUS)
        val = node.get_value()
        print(f"[SERVER STATUS]: {val}")
        #log_to_file(f"Server Status: {val}")
    except Exception as e:
        print(f"Error while trying to read status: {e}")

#switch inputs and outputs - commands: in <1-2> out <1-16>
def switch_inputs(client, node_command_id, out_choose):
    node_command = client.get_node(node_command_id)
    node_command.set_value(ua.Variant(out_choose, ua.VariantType.Int32))
    read_server_status(client)
    return True

#other commands e.g. status, clear...
def string_commands(client, command):
    node_status = client.get_node(NODE_STATUS)
    node_status.set_value(ua.Variant(command, ua.VariantType.String))
    read_server_status(client)
    return True

#main function
def main():
    #OPC client setup
    client = Client(SERVER_URL)
    client.set_user("")
    client.set_password("")

    print("============= OPC_UA MONITORING CLIENT =============")
    if not connect_with_retry(client):
        print("Fatal Error: Could not establish initial connection.")
        log_to_file("Fatal Error: Could not establish initial connection.")
        return
    
    print('Type "help" for usage')
    print('Press Ctrl+C to exit.')

    try:
        while True:
            try:
                users_input = input("> ").strip()
                if not users_input:
                    continue
                parts = users_input.split() #parser for commands
                
                #recognizing commands
                if len(parts) > 1:
                    #protection from unrecognizeg commanmds
                    if len(parts) < 3:
                        print('Wrong command. Type "help" to see examples.')
                        continue

                    in_choose = int(parts[1])
                    if in_choose < 1 or in_choose > 2:
                        print("Incorrect input. Available options: IN <1-2>")
                        continue
                    else:
                        #choosing input and getting oiutput assigned to other input
                        if int(in_choose) == 1:
                            node_command = client.get_node(NODE_COMMAND_1)
                            other_node = client.get_node(NODE_COMMAND_2)
                        elif int(in_choose) == 2:
                            node_command = client.get_node(NODE_COMMAND_2)
                            other_node = client.get_node(NODE_COMMAND_1)
                    other_output = other_node.get_value()

                    #shutting down input
                    if parts[2] == "off":
                        out_choose = -1
                        id_to_send = NODE_COMMAND_1 if in_choose == 1 else NODE_COMMAND_2
                        switch_inputs(client, id_to_send, out_choose)
                        continue

                    #protection from setting both inputs to same outputs
                    else:
                        out_choose = int(parts[3])
                        if out_choose == other_output:
                            print("Error. Cannot set the same outputs on both inputs!")
                            continue

                    #choosing outputs
                    if out_choose < 1 or out_choose > 16:
                        print("Incorrect output. Available options: OUT <1-16>")
                        continue
                    else:
                        switch_inputs(client, node_command, out_choose)

                #protection from unrecognized one-word commands
                if len(parts) == 1:
                    if parts[0] == "help":
                        print_help()
                    elif (parts[0] != "release" and parts[0] != "clear"
                          and parts[0] != "draw" and parts[0] != "screen"
                          and parts[0] != "status"):
                        print('Incorrect command. Type "help" for usage.')
                        continue

                    #operating short commands, e. g. status, clear
                    else:
                        string_commands(client, users_input)

            #operating network and opc errors
            except (OSError, socket.error, ConnectionError, ua.UaError, TimeoutError) as e:
                log_to_file(f"Connection lost or timeout: {e}")
                print(f"\n[!] Connection lost: {e}. Reconnecting...")
                try:
                    client.disconnect()
                except:
                    pass
                if not connect_with_retry(client):
                    print("Fatal Error: Could not reconnect after multiple attempts.")
                    break
                else:
                    print("Reconnected! Please repeat your command.")
                    continue 
            
            #other errors
            except Exception as e:
                log_to_file(f"General Error: {e}")
                print(f"An unexpected error occurred: {e}")

    except KeyboardInterrupt:
        print("\nApplication stopped by user.")

    #ending program
    finally:
        try:
            print("Closing session...")
            client.close_session()
            client.disconnect()
            
        except:
            pass
        print("Program ended")
if __name__ == "__main__":
    main()