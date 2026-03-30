import pandas as pd

# Load protocol hierarchy CSV
df = pd.read_csv("protocol_hierarchy.csv", skiprows=1, header=None)
df = df[0].str.split(',', expand=True)

df.columns = ["Protocol","PercentPackets","Packets","PercentBytes","Bytes",
              "BitsPerSec","EndPackets","EndBytes","EndBitsPerSec","PDUs"]

# Convert numeric columns
for col in ["Packets","Bytes"]:
    df[col] = pd.to_numeric(df[col], errors='coerce').fillna(0)

# 1️⃣ Total packets → only Frame row
total_packets = int(df[df["Protocol"].str.contains("Frame", case=False)]["Packets"].values[0])

# 2️⃣ Total captured bytes → Frame row
total_bytes = int(df[df["Protocol"].str.contains("Frame", case=False)]["Bytes"].values[0])

# 3️⃣ Data size (payload)
# Payload usually under Data / TLS Application Data / HTTP
data_bytes = int(df[df["Protocol"].str.contains("Data|Application|HTTP|TLS", case=False)]["Bytes"].sum())

# 4️⃣ Header size
header_bytes = total_bytes - data_bytes

print("Total number of packets:", total_packets)
print("Size of data:", data_bytes, "Bytes")
print("Size of headers:", header_bytes, "Bytes")
