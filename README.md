# Context-Adaptive-Huffman-Encoder

### 사용방법
encoder의 실행파일과 동일한 디렉토리에 텍스트파일<test_input1.txt / test_input2.txt / test_input3.txt / training_input.txt test_input1.txt>을 임의로 생성한 뒤 인코딩하면, 5개의 hbs파일<huffman_code1_code.hbs / test_input2_code.txt / test_input3_code.txt / huffman_table.hbs / context_adaptive_huffman_table.hbs>이 생성됩니다. (2개의 reference data와 압축데이터) 해당 파일을 decoder 실행파일과 동일한 디렉토리에 저장한 뒤, decoder를 실행시키면 원본파일로 복원이 가능합니다.

### 압축
<img width="525" alt="스크린샷 2022-06-10 오전 12 51 47" src="https://user-images.githubusercontent.com/43588644/172890713-9f16c746-cecf-469f-97ac-9311b05173f7.png">

원본파일의 30~40%대의 크기까지 압축이 가능합니다. 

### 테이블생성
<img width="527" alt="스크린샷 2022-06-10 오전 12 54 11" src="https://user-images.githubusercontent.com/43588644/172891185-c12ffd5b-21ed-4739-9247-ec22402f8463.png">
<img width="524" alt="스크린샷 2022-06-10 오전 12 54 37" src="https://user-images.githubusercontent.com/43588644/172891270-00329a19-2c12-43be-85cd-2525c93f4c3e.png">
