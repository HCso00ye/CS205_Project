# Audio Convertor : RAW, WAV, and FLAC

> 冯海波 12010505  A (significant contribution)
>
> 郑皓文 12110925  A (significant contribution)
>
> github：[HCso00ye/CS205_Project (github.com)](https://github.com/HCso00ye/CS205_Project)

## **Overview**

In this project, we implement a library in C/C++ that supports encoding conversion from pcm `.raw` audio files to `.wav` files and then to simple `.flac` files, as well as decoding wav format files. A Command Line Interface (CLI) is implemented to demonstrate the project.

### Audio Formats

1. **PCM (Raw):** Uncontainerized and uncompressed audio with raw pulse code modulation values.
2. **WAV (Waveform Audio File Format):** Microsoft's RIFF specification for storing digital audio files.
3. **FLAC (Free Lossless Audio Codec):** Lossless audio compression format, similar to MP3 but without any loss in quality.

### basic functions 

- [x] **FLAC Metadata Display & Edit** :  decode and display the information of metadata blocks of type 0 (STREAMINFO) and type 4 (VORBIS_COMMENT).
- [x] **Encoder : RAW to WAV**, using additional information saved in the `.ini` file.
- [x] **Encoder : WAV to FLAC** , with depth be **16Bit int** or **24Bit int**, encode subframe using **verbatim predictor** with channel assignment of **2 channels: left**, with **STREAMINFO** and **VORBIS_COMMENT** metadata.
- [x] **Decoder : WAV to RAW**: Further decoded .wav files to .raw format.
- [ ] **Decoder: FLAC to WAV**: Implemented a decoder to convert .flac files to .wav format.

### Bonus parts

- [x] Excellent Project Management:
  - Manage projects using the github repository

## Project Achievements

### In-Depth Understanding of Audio Formats

 In this project, I gained a profound understanding of the conversion process between raw audio files (PCM), WAV, and FLAC formats. I learned how to handle different formats of audio data and metadata. This deepened my comprehension of binary files, particularly in terms of how to process them. Understanding the encoding and decoding processes of audio data became more insightful.

### Enhancement of C/C++ Programming Skills

Through practical coding work in the project, my understanding of the C/C++ languages deepened, and my programming skills significantly improved. Especially during the manipulation of audio data, I honed my ability to operate on low-level data. The significance of using structs for effective data integration became more apparent. Analyzing and problem-solving skills were also elevated.

### Project Management and Team Collaboration

I learned how to organize code repositories, use version control effectively, and write user-friendly documentation. Team collaboration was facilitated through the GitHub platform. Understanding the importance of standardized code styles, I aim to establish clear conventions in future projects to enhance team collaboration efficiency.

### Rigorous Code Testing

The project emphasized the importance of timely testing and proper exception handling. Debugging challenges highlighted the pain associated with debugging, fostering a habit of systematic testing and debugging. Ensuring code correctness was recognized as a cornerstone for project success.

## Reflection and Improvement

### Encountered Challenges

Challenges were faced in handling metadata, primarily due to insufficient understanding of the FLAC format. These issues were addressed by implementing error-handling mechanisms (logging and exception throwing) and conducting thorough testing. Reading and understanding the details of the FLAC format gradually resolved these challenges.

### Solutions Implemented

To address challenges, error-handling mechanisms and extensive testing were employed. In-depth comprehension of the FLAC format details was achieved through continuous examination of the source code. Anticipating such challenges earlier and taking corresponding measures will be a valuable lesson for future projects.

### Importance of Documentation

In team collaboration, merging code written by two individuals with different styles proved cumbersome. To alleviate this, planning to establish coding styles and conventions beforehand is crucial for smoother integration. Additionally, enhancing the detail level of documentation will aid other developers in understanding and utilizing the library.

### User-Friendliness

Despite optimizations in various aspects, there is room for improvement in the design and interaction of the Command Line Interface (CLI). Future projects will focus on enhancing user-friendliness, including designing more intuitive user interfaces and providing detailed help documentation.

### Modularity and extensibility of code :

Code should be organized into clearly defined modules, each with a clear responsibility. If there are more audio formats or coding options, the existing architecture should allow for easy extension.

### Decoder: FLAC to WAV

In this project, a specific method was not implemented due to failure in time management, resulting in insufficient time to address unforeseen circumstances.

The overall concept has been outlined, but the code is still plagued with bugs, preventing it from running correctly.

The proposed method involves the following steps:

1. **Verify FLAC File:**
   - Check if the input file is a FLAC file.
2. **Metadata Handling:**
   - Utilize a struct to store metadata blocks of the FLAC file, specifically focusing on type 0 (STREAMINFO).
3. **WAV Header Construction:**
   - Construct the WAV header format.
4. **Convert Metadata to WAV Format:**
   - Transform metadata blocks of the FLAC file into WAV format, preserving them with the WAV header.
5. **Frame-by-Frame Conversion:**
   - Design a method to convert each frame of FLAC data to WAV data.
6. **Write WAV Data:**
   - Finally, write the WAV data to the output file.

While the overall approach has been conceptualized, the implementation phase was hindered by time constraints and unresolved bugs. The method aims to convert FLAC files to WAV format while preserving essential metadata and adhering to WAV header specifications. Addressing the existing bugs and refining the code is essential for the successful execution of this method. In future projects, proper time allocation and effective bug resolution strategies will be crucial for the smooth implementation of planned features.

### I was not familiar with the storage form of audio files and felt unable to start.

Carefully comb through the file format, understand the file structure, and compare with the files generated by the tool to find the cause of the problem.