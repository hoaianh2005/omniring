def KSA(S, K):
    j = 0
    N = len(S)
    key_len = len(K)

    for i in range(N):
        j = (j + S[i] + K[i % key_len]) % N
        S[i], S[j] = S[j], S[i]
    return S


def PRGA(S, n):
    i = 0
    j = 0
    N = len(S)
    key_stream = []

    for _ in range(n):
        i = (i + 1) % N
        j = (j + S[i]) % N

        S[i], S[j] = S[j], S[i]

        t = (S[i] + S[j]) % N
        key_stream.append(S[t])

    return key_stream


def rc4_encrypt(plaintext, key):
    # Khởi tạo S
    S = list(range(10))

    # KSA
    S = KSA(S, key.copy())

    # PRGA
    key_stream = PRGA(S, len(plaintext))

    # Chuyển plaintext sang ASCII
    m = [ord(c) for c in plaintext]

    # XOR
    cipher = [m[i] ^ key_stream[i] for i in range(len(m))]

    return cipher, key_stream


if __name__ == "__main__":
    plaintext = "cybersecurity"
    key = [2, 4, 1, 7]

    cipher, key_stream = rc4_encrypt(plaintext, key)

    print("Key stream:", key_stream)
    print("Cipher:", cipher)