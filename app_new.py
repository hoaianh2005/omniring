import streamlit as st
import math
import matplotlib.pyplot as plt
import random
import string
from zxcvbn import zxcvbn

# --- HÀM TÍNH TOÁN (Giữ nguyên) ---
def calculate_metrics(password):
    L = len(password)
    has_lower = any(c.islower() for c in password)
    has_upper = any(c.isupper() for c in password)
    has_digit = any(c.isdigit() for c in password)
    has_special = any(not c.isalnum() for c in password)
    
    N = 0
    if has_lower: N += 26
    if has_upper: N += 26
    if has_digit: N += 10
    if has_special: N += 32 
    
    if N == 0: theoretical_entropy = 0
    else: theoretical_entropy = L * math.log2(N)

    # Tính Entropy thực tế (zxcvbn)
    result = zxcvbn(password)
    guesses = result['guesses']
    real_entropy = math.log2(guesses) if guesses > 1 else 0
    crack_time_display = result['crack_times_display']['offline_slow_hashing_1e4_per_second']
    feedback = result['feedback']['warning']
    
    return theoretical_entropy, real_entropy, N, L, crack_time_display, feedback

# --- HÀM SINH MẬT KHẨU (Mới) ---
def generate_password(length, use_digits, use_special):
    characters = string.ascii_letters # Mặc định có chữ hoa + thường
    if use_digits:
        characters += string.digits
    if use_special:
        characters += string.punctuation
    
    # Chọn ngẫu nhiên
    return ''.join(random.choice(characters) for i in range(length))

# --- GIAO DIỆN WEB ---
st.set_page_config(page_title="Password Entropy Master", layout="wide")

# -- SIDEBAR: CÔNG CỤ SINH MẬT KHẨU --
st.sidebar.title("🛠️ Công cụ sinh Mật khẩu")
st.sidebar.markdown("Tạo mật khẩu ngẫu nhiên để đạt Entropy tối đa.")

gen_length = st.sidebar.slider("Độ dài mong muốn", 8, 32, 12)
use_digits = st.sidebar.checkbox("Bao gồm Số (0-9)", value=True)
use_special = st.sidebar.checkbox("Bao gồm Ký tự đặc biệt (!@#)", value=True)

if st.sidebar.button("⚡ TẠO & PHÂN TÍCH NGAY"):
    # Tạo mật khẩu và lưu vào session_state
    generated_pass = generate_password(gen_length, use_digits, use_special)
    st.session_state['password_input'] = generated_pass

# -- MAIN AREA --
st.title("🔐 Phân tích Entropy & Độ mạnh Mật khẩu")
st.markdown("---")

col1, col2 = st.columns([1, 1.5])

with col1:
    st.subheader("1. Nhập liệu")
    
    # Kiểm tra session state để điền giá trị mặc định
    default_val = st.session_state.get('password_input', '')
    
    password = st.text_input("Mật khẩu của bạn (hoặc tạo từ menu trái):", 
                             value=default_val, 
                             type="default", # Để type='password' nếu muốn che
                             placeholder="Nhập thử: 123456")
    
    if password:
        t_entropy, r_entropy, pool, length, time_crack, warning = calculate_metrics(password)
        
        st.divider()
        st.markdown("### 📊 Kết quả")
        
        # Metrics
        c1, c2 = st.columns(2)
        c1.metric("Entropy LÝ THUYẾT", f"{t_entropy:.1f} bits", help="Max Entropy (Hartley)")
        c2.metric("Entropy THỰC TẾ", f"{r_entropy:.1f} bits", 
                  delta=f"{r_entropy - t_entropy:.1f} bits",
                  delta_color="off" if abs(r_entropy - t_entropy) < 1 else "normal")
        
        # Nhận xét độ lệch
        # diff = t_entropy - r_entropy
        # if diff < 1:
        #     st.success("🌟 TUYỆT VỜI! Mật khẩu này ngẫu nhiên hoàn hảo. Entropy thực tế đạt mức tối đa.")
        # elif diff < 10:
        #     st.warning("Khá tốt, nhưng vẫn có chút quy luật.")
        # else:
        #     st.error(f"CẢNH BÁO: Mật khẩu này quá dễ đoán! Bạn mất {diff:.1f} bits do thói quen đặt pass.")

        if warning:
            st.error(f"⚠️ Phát hiện điểm yếu: **{warning}**")

        st.info(f"⏳ Thời gian Crack ước tính: **{time_crack}**")

with col2:
    if password:
        st.subheader("2. Biểu đồ Tương quan")
        fig, ax = plt.subplots(figsize=(8, 5))
        
        # Đường lý thuyết Max
        x = range(1, 35)
        y_max = [l * math.log2(94) for l in x] 
        ax.plot(x, y_max, color='lightgray', linestyle='--', label='Lý thuyết (Max Random)')
        
        # Điểm hiện tại
        ax.scatter([length], [t_entropy], color='blue', s=100, label='Entropy Lý thuyết', alpha=0.6)
        ax.scatter([length], [r_entropy], color='red', s=100, label='Entropy Thực tế', zorder=10)
        
        # Vẽ nối nếu lệch nhau
        if t_entropy - r_entropy > 1:
            ax.plot([length, length], [t_entropy, r_entropy], color='red', linestyle='dotted')
            ax.text(length + 0.5, (t_entropy+r_entropy)/2, "Sụt giảm", color="red")

        # Vùng an toàn
        # ax.axhline(y=60, color='green', linestyle='-', alpha=0.3)
        # ax.fill_between(x, 60, 250, color='green', alpha=0.05, label="Vùng An toàn (>60 bits)")

        ax.set_title(f"Độ mạnh: {pool} ký tự trong không gian mẫu")
        ax.set_xlabel("Độ dài")
        ax.set_ylabel("Entropy (bits)")
        ax.legend(loc='upper left')
        ax.grid(True, alpha=0.3)
        st.pyplot(fig)