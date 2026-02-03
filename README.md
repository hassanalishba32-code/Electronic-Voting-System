# Electronic Voting System (GUI)

## Purpose
- Simple GUI to register, login, vote, and view results

## Data Files
- `voting_data/data_encrypted.txt` = stored data (hex + XOR)
- `voting_data/data_decrypted.txt` = readable mirror

## TXT Data Structure
- One user per line
- Fields order = `CNIC|PASSWORD_HASH|VOTED|VOTED_FOR`
- VOTED = 1 (yes) or 0 (no)
- VOTED_FOR = candidate index (0, 1, 2) or -1 (if not voted)
- Example: `1234567890123|a1b2c3d4e5f6a7b8|1|2`

## Limits (Numbers)
- Users max = 100
- Candidates = 3
- CNIC length = 13 digits

## Install Needed
- C++ compiler (clang or g++)
- Qt 5 or Qt 6 (Core, Gui, Widgets)
- CMake (if you use a build system)

## Flow (Method)
- Register → save encrypted + decrypted
- Login → check CNIC + password hash
- Vote → add 1 to selected candidate
- Admin → view counts

## Vote Counts (Example Math)
- Total votes = A + B + C
- If A=2, B=1, C=0 → total = 3
