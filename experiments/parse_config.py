import sys
import json

# Load JSON file
config_file = sys.argv[1]
with open(config_file) as f:
    config = json.load(f)

# Extract and print variables
print(config.get('executable', ''), end=';')
print(','.join(map(str, config.get('num_threads', []))), end=';')
print(','.join(map(str, config.get('length', []))), end=';')
print(','.join(map(str, config.get('nset', []))), end=';')
print(','.join(map(str, config.get('vaccel', []))), end=';')
