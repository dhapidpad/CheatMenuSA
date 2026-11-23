"""
CheatMenuSA - Model Context Protocol (MCP) Python Client / Bridge
Connects to CheatMenuSA.asi running inside GTA San Andreas via TCP port 13337.

Usage:
  python scripts/mcp_client.py ping
  python scripts/mcp_client.py state
  python scripts/mcp_client.py teleport 2491.5 -1668.2 13.3
  python scripts/mcp_client.py spawn 411
  python scripts/mcp_client.py offsets
  python scripts/mcp_client.py logs
  python scripts/mcp_client.py cheat godmode true
"""

import socket
import json
import sys

MCP_HOST = "127.0.0.1"
MCP_PORT = 13337

def send_command(payload: dict) -> dict:
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5.0)
        s.connect((MCP_HOST, MCP_PORT))
        
        msg = json.dumps(payload) + "\n"
        s.sendall(msg.encode("utf-8"))
        
        data = s.recv(65536).decode("utf-8")
        s.close()
        
        return json.loads(data)
    except ConnectionRefusedError:
        return {"status": "error", "message": f"Could not connect to {MCP_HOST}:{MCP_PORT}. Is GTA San Andreas running with CheatMenuSA.asi?"}
    except Exception as e:
        return {"status": "error", "message": str(e)}

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return

    cmd = sys.argv[1].lower()

    if cmd == "ping":
        res = send_command({"action": "ping"})
        print(json.dumps(res, indent=2))

    elif cmd == "state":
        res = send_command({"action": "get_player_state"})
        print(json.dumps(res, indent=2))

    elif cmd == "teleport":
        if len(sys.argv) < 5:
            print("Usage: python mcp_client.py teleport <x> <y> <z>")
            return
        x, y, z = float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4])
        res = send_command({"action": "teleport", "x": x, "y": y, "z": z})
        print(json.dumps(res, indent=2))

    elif cmd == "spawn":
        model_id = int(sys.argv[2]) if len(sys.argv) > 2 else 411
        res = send_command({"action": "spawn_vehicle", "model_id": model_id})
        print(json.dumps(res, indent=2))

    elif cmd == "offsets":
        res = send_command({"action": "scan_offsets"})
        print(json.dumps(res, indent=2))

    elif cmd == "logs":
        count = int(sys.argv[2]) if len(sys.argv) > 2 else 20
        res = send_command({"action": "get_logs", "count": count})
        print(json.dumps(res, indent=2))

    elif cmd == "cheat":
        if len(sys.argv) < 4:
            print("Usage: python mcp_client.py cheat <name> <true/false>")
            return
        name = sys.argv[2]
        enabled = sys.argv[3].lower() in ("true", "1", "yes")
        res = send_command({"action": "set_cheat", "name": name, "enabled": enabled})
        print(json.dumps(res, indent=2))

    else:
        print(f"Unknown command: {cmd}")
        print(__doc__)

if __name__ == "__main__":
    main()
