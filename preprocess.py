import argparse
import os
import sys
import multiprocessing
from tqdm import tqdm

# --- FIX 1: Define a global variable for the mapping dictionary ---
# This will be populated by the main process and inherited by child processes.
mapping_dict = {}

def get_file_chunks(file_path, num_chunks):
    # This function remains the same as before
    try:
        file_size = os.path.getsize(file_path)
    except FileNotFoundError:
        print(f"Error: Input file not found '{file_path}'", file=sys.stderr)
        sys.exit(1)
    
    chunk_size = file_size // num_chunks
    chunks, start = [], 0
    for i in range(num_chunks):
        end = start + chunk_size
        if i < num_chunks - 1:
            with open(file_path, 'rb') as f:
                f.seek(end)
                f.readline()
                end = f.tell()
        else:
            end = file_size

        chunks.append((file_path, start, end))
        start = end
        if start >= file_size: break
    return chunks

def discover_ids_chunk(args):
    # This function remains the same as before
    file_path, start, end = args
    unique_ids = set()
    with open(file_path, 'r', encoding='utf-8') as f:
        f.seek(start)
        while f.tell() < end:
            line = f.readline()
            if not line: break
            try:
                src, dst = line.split()
                unique_ids.add(int(src))
                unique_ids.add(int(dst))
            except ValueError:
                continue
    return unique_ids

# --- FIX 2: Create an initializer function for worker processes ---
def init_worker(the_dict):
    """This runs once per child process. It sets the global variable."""
    global mapping_dict
    mapping_dict = the_dict

def remap_ids_chunk(args):
    """Worker function for Pass 2. Now uses the global mapping_dict."""
    # --- FIX 3: The large dictionary is no longer passed as an argument ---
    file_path, start, end, temp_file_path = args # No more 'original_to_compact'
    with open(file_path, 'r', encoding='utf-8') as f_in, open(temp_file_path, 'w', encoding='utf-8') as f_out:
        f_in.seek(start)
        while f_in.tell() < end:
            line = f_in.readline()
            if not line: break
            try:
                src, dst = line.split()
                # Access the dictionary from the global scope
                compact_src = mapping_dict[int(src)]
                compact_dst = mapping_dict[int(dst)]
                f_out.write(f"{compact_src} {compact_dst}\n")
            except (ValueError, KeyError):
                continue
    return temp_file_path

def run_preprocessing():
    # ... (parser setup remains the same)
    parser = argparse.ArgumentParser(description="Preprocess graph data in parallel for PageRank benchmark.")
    parser.add_argument("input_file", help="Path to the original graph file (e.g., links-anon.txt).")
    parser.add_argument("-o", "--output_graph", default="links-preprocessed.txt", help="Path to save the preprocessed graph file.")
    parser.add_argument("-m", "--output_mapping", default="id_mappings.txt", help="Path to save the ID mapping file.")
    parser.add_argument("-p", "--processes", type=int, default=os.cpu_count(), help="Number of processes to use. Defaults to all available CPU cores.")
    args = parser.parse_args()
    num_processes = args.processes
    print(f"Using {num_processes} processes for parallel execution.")

    # --- Pass 1 is unchanged ---
    print(f"--- Pass 1: Discovering unique node IDs from '{args.input_file}' in parallel...")
    chunks = get_file_chunks(args.input_file, num_processes)
    with multiprocessing.Pool(processes=num_processes) as pool:
        results = list(tqdm(pool.imap(discover_ids_chunk, chunks), total=len(chunks), desc="Scanning Chunks"))
    
    print("Merging results from all processes...")
    original_to_compact = {original_id: compact_id for compact_id, original_id in enumerate(sorted(list(set.union(*results))))}
    num_unique_nodes = len(original_to_compact)
    print(f"Discovered {num_unique_nodes} unique nodes.")
    
    # --- Pass 2 is now memory-efficient ---
    print(f"--- Pass 2: Remapping edges and writing to temporary files in parallel...")
    temp_dir = "temp_chunks"
    if not os.path.exists(temp_dir):
        os.makedirs(temp_dir)
        
    temp_files = [os.path.join(temp_dir, f"chunk_{i}.txt") for i in range(len(chunks))]
    # The large dictionary is now removed from the arguments
    remap_args = [(chunks[i][0], chunks[i][1], chunks[i][2], temp_files[i]) for i in range(len(chunks))]
    
    # --- FIX 4: Use the initializer when creating the Pool ---
    with multiprocessing.Pool(processes=num_processes, initializer=init_worker, initargs=(original_to_compact,)) as pool:
        list(tqdm(pool.imap(remap_ids_chunk, remap_args), total=len(remap_args), desc="Remapping Chunks"))

    # ... (Concatenating and cleanup code remains the same) ...
    print(f"Concatenating temporary files into '{args.output_graph}'...")
    with open(args.output_graph, 'wb') as f_out:
        for temp_file in tqdm(temp_files, desc="Merging Files"):
            with open(temp_file, 'rb') as f_in:
                f_out.write(f_in.read())
            os.remove(temp_file)
    os.rmdir(temp_dir)

    print(f"Saving ID mappings to '{args.output_mapping}'...")
    with open(args.output_mapping, 'w', encoding='utf-8') as f_map:
        f_map.write("compact_id,original_id\n")
        lines_to_write = [f"{i},{original_id}\n" for original_id, i in original_to_compact.items()]
        f_map.writelines(lines_to_write)
        
    print("\n" + "="*50)
    print("✅ Parallel preprocessing complete!")
    print("="*50)
    print("\nRun your C++ benchmark with the following parameters:")
    print(f"  Max ID (Node Count): {num_unique_nodes}")
    print(f"  Line Num (Edge Count): (Please use the known value from your dataset)")
    print(f"  Input File: {args.output_graph}")
    print("-" * 50)

if __name__ == "__main__":
    run_preprocessing()