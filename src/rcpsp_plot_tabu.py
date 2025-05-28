import pandas as pd
import matplotlib.pyplot as plt

# === WYKRES GANTTA ===

def rysuj_gantta(plik_csv):
    df = pd.read_csv(plik_csv)
    df = df.sort_values(by="start_time")

    fig, ax = plt.subplots(figsize=(12, 8))

    for _, row in df.iterrows():
        ax.barh(
            y=row["job_id"],
            width=row["end_time"] - row["start_time"],
            left=row["start_time"],
            height=0.8,
            align='center',
            color='steelblue'
        )
        ax.text(
            row["start_time"] + (row["end_time"] - row["start_time"]) / 2,
            row["job_id"],
            f'{row["job_id"]}',
            va='center',
            ha='center',
            color='white',
            fontsize=8,
            fontweight='bold'
        )

    ax.set_xlabel("Czas")
    ax.set_ylabel("Zadanie (job_id)")
    ax.set_title("Wykres Gantta – Harmonogram RCPSP")
    ax.grid(True)
    plt.tight_layout()
    plt.show()

# === WYKRES ZASOBÓW ===

def rysuj_zasoby(plik_csv):
    df = pd.read_csv(plik_csv)
    czas = df["czas"]
    zasoby = df.drop(columns=["czas"])

    fig, axes = plt.subplots(nrows=zasoby.shape[1], figsize=(12, 2.5 * zasoby.shape[1]), sharex=True)

    if zasoby.shape[1] == 1:
        axes = [axes]

    for i, col in enumerate(zasoby.columns):
        axes[i].plot(czas, zasoby[col], drawstyle='steps-post')
        axes[i].set_ylabel(col)
        axes[i].grid(True)

    axes[-1].set_xlabel("Czas")
    plt.suptitle("Wykorzystanie zasobów w czasie", fontsize=14)
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()

# === URUCHOMIENIE ===

if __name__ == "__main__":
    rysuj_gantta("harmonogram_tabu.csv")
    rysuj_zasoby("zasoby_tabu.csv")
