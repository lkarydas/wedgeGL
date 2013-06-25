CC = g++
FILES = orangeSlice.cpp InitShader.cpp
OUT_EXE = orangeSlice
LIBRARIES = -lGLEW -lglut -lGLU

build: $(FILES)
	$(CC) -o $(OUT_EXE) $(FILES) $(LIBRARIES) && ./$(OUT_EXE)

clean:
	rm -fv *.o core
	rm -fv $(OUT_EXE)

rebuild: clean build