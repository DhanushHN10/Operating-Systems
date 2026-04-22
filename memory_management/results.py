import subprocess

# === Configuration ===
executable = "./FIFO_OPTIMAL.out"

l = [[64,4], [64, 128], [128, 1024], [1024,128], [4096,1024], [16384,4096], [65536,16384], [65536, 65536]] # pairs of page size and number of frames
replacement_policies = ["FIFO"]  # optional — can limit to ["FIFO"]
allocation_policies = ["LOCAL", "GLOBAL"]   # optional — can limit to one
trace_file = "combined.trace"
output_file = "results.txt"

# === Run experiments ===
with open(output_file, "w") as out:
    for repl in replacement_policies:
        for alloc in allocation_policies:
            # for ps in page_sizes:
            #     for nf in num_frames_list:
            for ps, nf in l:
            
                    cmd = [
                        executable,
                        str(ps),
                        str(nf),
                        repl,
                        alloc,
                        trace_file
                    ]
                    header = (
                        f"=== PAGE_SIZE={ps}, FRAMES={nf}, "
                        f"REPLACEMENT={repl}, ALLOCATION={alloc} ===\n"
                    )
                    print(f"Running: {header.strip()}")
                    out.write(header)

                    # Run the command and capture output
                    result = subprocess.run(cmd, capture_output=True, text=True)

                    # Write both stdout and stderr to the output file
                    out.write(result.stdout)
                    if result.stderr:
                        out.write("\n--- STDERR ---\n")
                        out.write(result.stderr)
                    out.write("\n\n")

print(f"\n✅ All experiment results saved to {output_file}")
