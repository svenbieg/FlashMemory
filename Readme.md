<h1>Flash-Memory</h1>

<p>
This is the construction-site of my <a href="https://github.com/svenbieg/Database">Database</a>.<br />
</p>

<img src="https://github.com/user-attachments/assets/b48e402e-5c09-4d0f-a77f-38048699f40d" width="500" /><br />

<h2>02. May 2026</h2>

<p>
I can test my implementation in a file on Windows and on real hardware.<br />
My 128MB unmanaged NAND flash memory from <a href="https://www.micron.com/products/storage/nand-flash">Micron</a> arrived today.
</p>

<img src="https://github.com/user-attachments/assets/e2f0fe1a-c010-4323-8439-c71e08f373a9" width="500" /><br />
<br />

<h2>17. May 2026</h2>

<p>
The chip is responding. Reading a page takes about 1ms (2MB/s). I've disabled <a href="https://github.com/svenbieg/Database/wiki/Storage#error-correction">error-correction</a> for my skip-bits, allowing me to skip whole pages out of date. 64 pages, 64 skip-bits.<br />
It really takes 8 bytes to append information reliably.
</p>

<img src="https://github.com/user-attachments/assets/fed2cd32-67d2-41df-bbb5-b57ed9258a3f" width="600" />
<br />
