#!/usr/bin/env python3
""" Main """
from time import sleep
from multiprocessing import Process, Queue
from args_handling import get_args
from trentorian import Trantorian


def burry_dead(pop: list[Process]) -> list[Process]:
    """remove all the dead trantorians from the population

    Args:
        pop (list[Process]): list of trantorians

    Returns:
        list[Process]: filtered list
    """
    return list(filter(lambda trant:trant.is_alive(), pop))


def main(host: str, port: int, team: str):
    """main for the ai

    Args:
        host (str): machine hosting serv
        port (int): port open for the serv
        team (str): name of the team
    """
    threads: list[Process] = []
    queue: Queue = Queue()
    t = Trantorian(host, port, team)
    threads.append(Process(target=t.born, args=(queue,)))

    for e in threads:
        e.start()

    while len(threads) > 0:
        threads = burry_dead(threads)
        if queue.empty():
            continue
        sended = queue.get()
        if sended.startswith("birth"):
            try:
                t = Trantorian(host, port, team)
            except RuntimeError:
                print("Trantor was aborted")
            threads.append(Process(target=t.born, args=(queue,)))
            threads[-1].start()
    return

def main_cli():
    args = get_args()

    # print(args.p)
    # print(args.n)
    # print(args.he)

    try:
        main(args.he, args.p, args.n)
    except RuntimeError as err:
        print(err.args[0])
        exit(84)

if __name__ == "__main__":
    main_cli()
