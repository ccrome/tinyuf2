with open("src/tinyuf2-binary.h", "w") as out:
    with open("../../../_build/teensy41/tinyuf2-teensy41.bin", "rb") as f:
        out.write("// tinyuf2 binary\n")
        all_bytes = f.read()
        out.write(f"uint32_t tinyuf2_length = {len(all_bytes)};")
        out.write("uint8_t tinyuf2_binary[] = {")
        out.write(", ".join([f"0x{x:02x}" for x in all_bytes]))
        out.write("};")
        
