## **Audio Convertor : RAW, WAV, and FLAC**

### **Overview**

In this project, we implement a library in C/C++ that supports encoding conversion from pcm `.raw` audio files to `.wav` files and then to simple `.flac` files, as well as decoding wav format files. A Command Line Interface (CLI) is implemented to demonstrate the project.





### basic functions 

- **FLAC Metadata Display & Edit** :  decode and display the information of metadata blocks of type 0 (STREAMINFO) and type 4 (VORBIS_COMMENT).

- **Encoder : RAW to WAV**, using additional information saved in the `.ini` file.

- **Encoder : WAV to FLAC** , with depth be **16Bit int** or **24Bit int**, encode subframe using **verbatim predictor** with channel assignment of **2 channels: left**, with **STREAMINFO** and **VORBIS_COMMENT** metadata.
- **Decoder : WAV to RAW**

### Bonus parts

- **Project Management**
  - We manage our project via GitHub.



### Contributions

- 











