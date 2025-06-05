import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx


def rysuj_zasoby(plik_csv):
    df = pd.read_csv(plik_csv)
    czas = df["czas"]
    zasoby = df.drop(columns=["czas"])

    fig, axes = plt.subplots(nrows=zasoby.shape[1], figsize=(12, 2.5 * zasoby.shape[1]), sharex=True, constrained_layout=True)

    if zasoby.shape[1] == 1:
        axes = [axes]

    for i, col in enumerate(zasoby.columns):
        axes[i].plot(czas, zasoby[col], drawstyle='steps-post')
        axes[i].set_ylabel(col)
        axes[i].grid(True)

    axes[-1].set_xlabel("Czas")
    plt.suptitle("Wykorzystanie zasobów w czasie", fontsize=14)
    plt.show()


def znajdz_sciezke_krytyczna(df, predecessors):
    najwczesniejszy_koniec = {}
    sciezki = {}

    for idx, row in df.iterrows():
        job = row["job_id"]
        preds = predecessors.get(job, [])
        if not preds:
            najwczesniejszy_koniec[job] = row["duration"]
            sciezki[job] = [job]
        else:
            najlepszy_pred = max(preds, key=lambda p: najwczesniejszy_koniec[p])
            najwczesniejszy_koniec[job] = najwczesniejszy_koniec[najlepszy_pred] + row["duration"]
            sciezki[job] = sciezki[najlepszy_pred] + [job]

    koniec = max(najwczesniejszy_koniec, key=najwczesniejszy_koniec.get)
    return sciezki[koniec]


def rysuj_graf(df):
    G = nx.DiGraph()

    predecessors = {}
    for _, row in df.iterrows():
        job = row["job_id"]
        raw = row.get("predecessors", "")
        if pd.isna(raw) or str(raw).strip() == "":
            preds = []
        else:
            preds = list(map(int, str(raw).strip().split()))
        predecessors[job] = preds
        G.add_node(job, start=row["start_time"])
        for p in preds:
            G.add_edge(p, job)

    sciezka = znajdz_sciezke_krytyczna(df, predecessors)

    pos = nx.spring_layout(G, seed=42)
    colors = ["red" if node in sciezka else "skyblue" for node in G.nodes()]
    labels = {node: f"{node}\n({int(df[df.job_id == node]['start_time'].iloc[0])})" for node in G.nodes()}

    plt.figure(figsize=(12, 8))
    nx.draw(G, pos, with_labels=True, node_color=colors, labels=labels, node_size=1000, font_size=9, font_weight='bold')
    nx.draw_networkx_edges(G, pos, edgelist=G.edges(), arrows=True)
    plt.title("Graf zależności zadań (kolor czerwony = ścieżka krytyczna)")
    plt.axis('off')
    plt.tight_layout()
    plt.show()

    rysuj_graf_sciezki_krytycznej(df, sciezka)


def rysuj_graf_sciezki_krytycznej(df, sciezka):
    G = nx.DiGraph()
    indeksy = set(sciezka)

    for job in sciezka:
        raw = df[df.job_id == job]["predecessors"].values[0]
        if pd.isna(raw) or str(raw).strip() == "":
            preds = []
        else:
            preds = list(map(int, str(raw).strip().split()))
        preds = [p for p in preds if p in indeksy]
        for p in preds:
            G.add_edge(p, job)
        G.add_node(job, start=df[df.job_id == job]["start_time"].iloc[0])

    pos = nx.spring_layout(G, seed=42)
    labels = {node: f"{node}\n({int(df[df.job_id == node]['start_time'].iloc[0])})" for node in G.nodes()}

    plt.figure(figsize=(10, 6))
    nx.draw(G, pos, with_labels=True, node_color="red", labels=labels, node_size=1000, font_size=9, font_weight='bold', edge_color="black")
    plt.title("Tylko ścieżka krytyczna")
    plt.axis('off')
    plt.tight_layout()
    plt.show()


def rysuj_gantta_z_zasobami(df):
    zasoby_cols = ['R1', 'R2', 'R3', 'R4']
    df["zasoby_opis"] = df[zasoby_cols].apply(
        lambda row: " ".join(f"{zasob}={int(row[zasob])}" for zasob in zasoby_cols if row[zasob] > 0),
        axis=1
    )
    df["zasoby_suma"] = df[zasoby_cols].sum(axis=1)

    fig, ax = plt.subplots(figsize=(12, 8))
    for _, row in df.iterrows():
        ax.barh(y=row["job_id"],
                width=row["duration"],
                left=row["start_time"],
                height=0.8,
                color=plt.cm.viridis(row["zasoby_suma"] / df["zasoby_suma"].max()))
        ax.text(row["start_time"] + row["duration"] / 2,
                row["job_id"],
                f'#{row["job_id"]} [{row["zasoby_opis"]}]',
                va='center', ha='center', fontsize=8, color='white')

    ax.set_xlabel("Czas")
    ax.set_ylabel("Zadanie")
    ax.set_title("Wykres Gantta z opisem zasobów (Greedy)")
    ax.invert_yaxis()
    plt.grid(True, axis='x', linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    df = pd.read_csv("harmonogram_greedy.csv")
    rysuj_graf(df)
    rysuj_zasoby("zasoby_greedy.csv")
    rysuj_gantta_z_zasobami(df)
