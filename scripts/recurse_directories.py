import os
import subprocess
import argparse
import sys


def run_command_on_files(
    directories: list[str], extensions: list[str], command: list[str]
) -> None:
    """Run the input command on all files with the specified extensions in the given directories."""
    for directory in directories:
        for root, _, files in os.walk(directory):
            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    file_path = os.path.join(root, file)
                    full_command = command + [file_path]
                    try:
                        print(f"Running: {' '.join(full_command)}")
                        subprocess.run(full_command, check=True)
                    except subprocess.CalledProcessError as e:
                        sys.exit(e.returncode)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-d",
        "--directories",
        nargs="+",
        required=True,
        help="List of directories to iterate over",
    )
    parser.add_argument(
        "-e",
        "--extensions",
        nargs="+",
        required=True,
        help="List of file extensions to target (e.g., .cpp .h)",
    )
    parser.add_argument(
        "-c",
        "--command",
        nargs=argparse.REMAINDER,
        required=True,
        help="Command to run",
    )

    args = parser.parse_args()

    if not args.command:
        parser.error("You must specify a command using -c or --command")

    run_command_on_files(args.directories, args.extensions, args.command)


if __name__ == "__main__":
    main()
