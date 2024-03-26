# npodbc

ODBC with NumPy

## Installation

Create a virtual environment using `conda` or `mamba`, activate it, and then install the
project. You will need the tool `bear` if you want to maintain a set of
`compile_commands.json` within the repo.

```bash
mamba env create --file environment
mamba activate npodbc-dev
bear -- pip install --no-build-isolation --editable .[dev,docs,test]
```

### SQL 2022

Run the following docker command to install a Microsoft SQL 2022 database, and start a
container.

```bash
sudo docker build . --tag mssql2022
sudo docker run -p 1401:1433 --name mssql2022 --hostname mssql2022 -m 16GB -d mssql2022
```

You can log into the running container if you want.

```bash
sudo docker exec -it mssql2022 bash
```

With the Docker container running you can now open an `ipython` session and import the
package.

```bash
ipython
```

```python
import npodbc
print(npodbc.driver_connect())
```
