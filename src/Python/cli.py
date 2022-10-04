from platform import node
import click
from . import russell as cantor


@click.group()
def main():
    pass

@main.command()
@click.option("--daemon", is_flag=True,
              show_default=True, default=False, help="run as daemon")
@click.option("--nodeserver","--AsNodeServer","--asNodeServer",
              "--nodeServer","--NodeServer", 
              is_flag=True,
              show_default=True, default=False, help="run as daemon")
@click.option('--port', default=7309, help='server port')
def start(daemon,nodeserver,port):
    print("start,port={},daemon={},AsNodeServer={}".format(port,daemon,nodeserver))
    if daemon:
        cantor.RunAsBackend(
            daemon=daemon,
            asNodeServer=nodeserver,
            port=port)

@main.command()
def test():
    cantor.InitClient()
    nodeName = cantor.KVGet("NodeName")
    print("\nNodeName=",nodeName)

@main.command()
@click.option('--script',help='script to run')
def run(script):
    cantor.InitClient()
    print("\nscript=",script,"\n")
    script = script.replace("\\n",'\n')
    script = script.replace("\\\\n",'\n')
    sc = compile(script,'cantor_script', 'exec')
    retVal = eval(sc)
    print(retVal)


@main.command()
def stop():
    """Command on cli2"""
    print("cmd2")

if __name__ == '__main__':
    main()

