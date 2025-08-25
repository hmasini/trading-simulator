import React, { useEffect, useState } from 'react';
import './App.css';

import { Chart } from "react-google-charts";

function CandlestickChart({ trades }) {
  // Group trades by second (or minute)
  const grouped = {};
  trades.forEach(trade => {
    // Use minute or second as key
    const key = trade.timestamp.slice(0, 19); // "YYYY-MM-DD HH:MM:SS"
    if (!grouped[key]) grouped[key] = [];
    grouped[key].push(Number(trade.price));
  });

  // Calculate OHLC for each group
  const ohlcData = Object.entries(grouped).map(([timestamp, prices]) => {
    return {
      x: timestamp,
      o: prices[0],
      h: Math.max(...prices),
      l: Math.min(...prices),
      c: prices[prices.length - 1]
    };
  });

  // Convert to Google Charts format, show only time
  let chartData = [
    ['Time', 'Low', 'Open', 'Close', 'High'],
    ...ohlcData.map(d => [
      d.x.slice(11), // Only HH:MM:SS
      d.l, d.o, d.c, d.h
    ])
  ];

  // Pad with empty intervals if less than 20 data points
  const minCandles = 20;
  while (chartData.length < minCandles + 1) { // +1 for header
    chartData.push([ '', null, null, null, null ]);
  }

  const options = {
    legend: 'none',
    backgroundColor: '#f7f7fa', // Off-white background
    candlestick: {
      fallingColor: { strokeWidth: 0, fill: '#f44336' },
      risingColor: { strokeWidth: 0, fill: '#4caf50' }
    },
    hAxis: { textStyle: { color: '#222' } },
    vAxis: { textStyle: { color: '#222' } }
  };

  return (
    <div className="candlestick-container" style={{ background: '#f7f7fa', color: '#222', borderRadius: '16px', padding: '2rem 2.5rem', margin: '2rem 0', boxShadow: '0 4px 24px rgba(76,175,80,0.06)' }}>
      <h2 style={{ color: '#4caf50', marginBottom: '1rem', fontWeight: 600 }}>Candlestick Chart</h2>
      <Chart
        chartType="CandlestickChart"
        width="100%"
        height="400px"
        data={chartData}
        options={options}
      />
    </div>
  );
}

function Card({ title, items, color }) {
  return (
    <div className="order-card" style={{ borderColor: color }}>
      <h2 style={{ color }}>{title}</h2>
      <ul>
        {items.map((item, idx) => (
          <li key={idx}>
            <span className="price">{item.price}</span>
            <span className="qty">Qty: {item.quantity}</span>
          </li>
        ))}
      </ul>
    </div>
  );
}

function App() {
  const [orderBooks, setOrderBooks] = useState([]);
  const [recentTrades, setRecentTrades] = useState([]);
  const [selectedSymbol, setSelectedSymbol] = useState(null);

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:9001');
    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        setOrderBooks(data.order_books || []);
        setRecentTrades(prevTrades => {
          const newTrades = (data.recent_trades || []);
          // Filter out trades already in prevTrades (by unique timestamp+symbol+price)
          const existing = new Set(prevTrades.map(t => `${t.symbol}-${t.timestamp}-${t.price}`));
          const filtered = newTrades.filter(t => !existing.has(`${t.symbol}-${t.timestamp}-${t.price}`));
          return [...prevTrades, ...filtered];
        });
      } catch (e) {
        console.error('Error parsing message:', e);
      }
    };
    ws.onclose = () => {
      console.log('WebSocket closed');
    };
    return () => ws.close();
  }, []);

  useEffect(() => {
    if (selectedSymbol === null && orderBooks.length > 0) {
      setSelectedSymbol(orderBooks[0].symbol);
    }
  }, [orderBooks, selectedSymbol]);

  const currentBook = orderBooks.find(ob => ob.symbol === selectedSymbol);
  const tradesForSymbolRaw = recentTrades.filter(t => t.symbol === selectedSymbol);
  const tradesForSymbol = tradesForSymbolRaw.slice(-400); // Limit to last 400 trades

  console.log("trades for symbol", tradesForSymbol.length);

  return (
    <div className="App" style={{ background: '#f7f7fa', color: '#222', minHeight: '100vh', padding: '2rem' }}>
      <h1 style={{
        textAlign: 'center',
        marginBottom: '2rem',
        fontSize: '2.5rem',
        letterSpacing: '2px',
        color: '#4caf50',
        fontWeight: 700
      }}>
        Trading Simulator
      </h1>
      <div className="tabs" style={{ display: 'flex', justifyContent: 'center', marginBottom: '2rem', gap: '1rem' }}>
        {orderBooks.map(ob => (
          <button
            key={ob.symbol}
            className={`tab-btn${selectedSymbol === ob.symbol ? ' active' : ''}`}
            style={{
              background: selectedSymbol === ob.symbol ? '#fff' : '#eaeaea',
              color: selectedSymbol === ob.symbol ? '#4caf50' : '#222',
              border: 'none',
              borderBottom: selectedSymbol === ob.symbol ? '3px solid #4caf50' : '3px solid transparent',
              padding: '0.75rem 2rem',
              borderRadius: '24px 24px 0 0',
              cursor: 'pointer',
              fontWeight: 700,
              fontSize: '1.1rem',
              boxShadow: '0 2px 8px rgba(0,0,0,0.04)',
              transition: 'background 0.2s, border-bottom 0.2s'
            }}
            onClick={() => setSelectedSymbol(ob.symbol)}
          >
            {ob.symbol}
          </button>
        ))}
      </div>
      {currentBook ? (
        <div className="orderbook-container">
          <Card title="Bids" items={currentBook.bids} color="#4caf50" />
          <Card title="Asks" items={currentBook.asks} color="#f44336" />
        </div>
      ) : (
        <p style={{ textAlign: 'center' }}>Waiting for data...</p>
      )}
      {
        tradesForSymbol.length > 20 ? (
          <CandlestickChart trades={tradesForSymbol} />
        ) : (
          <p style={{ textAlign: 'center' }}>Waiting for enough data points to display the candlestick chart.</p>
        )
      }
    </div>
  );
}

export default App;
