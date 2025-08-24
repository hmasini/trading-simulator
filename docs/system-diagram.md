```mermaid
flowchart TD
    subgraph Frontend
        A[React Frontend]
    end

    subgraph Backend
        B[WebSocket Server]
        C[Matching Engine]
        D[Order Book]
        E[Logger]
        F[CSV Writer]
        G[Order Types]
        H[Bots]
    end

    A <--> B
    B --> H
    B --> C
    H --> C
    C --> D
    C --> E
    C --> F
    C --> G
    G --> C