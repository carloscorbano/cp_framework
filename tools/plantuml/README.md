PlantUML helper

This folder contains a helper to download `plantuml.jar` locally so you don't have to install PlantUML system-wide.

Prerequisites:
- Java (JRE/JDK) must be available in PATH if you plan to run `plantuml.jar` via `java -jar`.

```cmd
cmake -S . -B build
cmake --build build --target doc_uml
```

This will use `java -jar tools/plantuml/plantuml.jar` if `plantuml` executable is not found in PATH.

If you prefer, download `plantuml.jar` manually from https://plantuml.com/download and place it in this folder.
