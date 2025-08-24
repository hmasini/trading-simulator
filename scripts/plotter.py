import matplotlib.pyplot as plt

symbol = "NVDA"
log_file = "../build/trading.log"

trade_prices = []

with open(log_file) as f:
    for line in f:
        parts = [p.strip() for p in line.strip().split(',')]
        if len(parts) != 7:
            continue
        event, buy_id, sell_id, sym, price, qty, time_str = parts
        if event != "TRADE" or sym != symbol:
            continue
        trade_prices.append(float(price))

fig, ax = plt.subplots()

# Set black background
ax.set_facecolor((0, 0, 0, 0.95))
fig.patch.set_facecolor((0, 0, 0, 0.95))

# Plot trade prices with a "trade chart" look
ax.plot(
    range(1, len(trade_prices)+1),
    trade_prices,
    color='cyan',
    linewidth=2,
    marker='o',
    markersize=6,
    markerfacecolor='lime',
    markeredgecolor='black',
    label=f'TRADE {symbol}'
)

# Add a horizontal line for average price
if trade_prices:
    avg_price = sum(trade_prices) / len(trade_prices)
    ax.axhline(avg_price, color='gray', linestyle='--', linewidth=1, alpha=0.7)
    ax.text(1, avg_price, f'Avg: {avg_price:.2f}', color='gray', va='bottom', ha='left', fontsize=10)

# Style axes and title
ax.set_title(f"Trade Prices for {symbol} (Sequential Index)", color='white', fontsize=14)
ax.set_xlabel("Trade Index", color='white')
ax.set_ylabel("Price", color='white')
ax.tick_params(axis='x', colors='white')
ax.tick_params(axis='y', colors='white')

# Add grid
ax.grid(True, color='gray', linestyle=':', alpha=0.3)

# Style legend
legend = ax.legend()
legend.get_frame().set_facecolor((0, 0, 0, 0))
legend.get_frame().set_edgecolor('none')
for text in legend.get_texts():
    text.set_color('white')

plt.tight_layout()
plt.show()