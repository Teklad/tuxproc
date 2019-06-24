## Test Program

Instructions for this demo:

 - compile both programs using the respective makefiles
 - start the dummy program `test` in a seperate shell session
 - press `<enter>` to display the current value of the target variable the `remote` program will modify
 - start the `remote` program with the PID of the dummy process as an argument, e.g. `remote $(pgrep test)`
 - press `<enter>` in the dummy program `test` again to display the new, modified value
