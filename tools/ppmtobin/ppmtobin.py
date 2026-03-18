import sys

def ppm_to_pristine(input_ppm, output_bin):
    with open(input_ppm, 'rb') as f:
        # skip P6 header
        assert f.readline().strip() == b'P6'
        # skip comments
        line = f.readline()
        while line.startswith(b'#'):
            line = f.readline()
        w, h = map(int, line.split())
        assert f.readline().strip() == b'255'
        pixels = f.read() # raw RGB bytes
    
    with open(output_bin, 'wb') as f:
        import struct
        f.write(struct.pack('<II', w, h))
        f.write(pixels)

if len(sys.argv) > 2:
    ppm_to_pristine(sys.argv[1], sys.argv[2])
else:
    print("Usage:", sys.argv[0], "input-ppm", "output-bin")