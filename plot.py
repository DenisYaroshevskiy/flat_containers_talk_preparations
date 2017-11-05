import argparse
import json
import plotly
import re
import itertools

from plotly.graph_objs import Scatter, Layout

last_40 = False

class Measurement:
  def __init__(self, input_size, time):
    self.input_size = input_size
    self.time = time

class Variation:
  def __init__(self, name, measurements):
    self.name = name
    self.measurements = measurements
    self.name = self.name.replace('_', ' ')


def parseMeasurement(json_dict):
  parsed_name = re.match(r'(.*?)(_solution)?/(.*)/(.*)',json_dict['name'])
  name = parsed_name.group(1)

  input_size = int(parsed_name.group(3))
  time = json_dict['real_time']
  return (name, Measurement(input_size, time))

def parseJson(loaded_benchmarks_json, baseline_name):
  measurements = {}
  for json_input in loaded_benchmarks_json['benchmarks']:
    name, m = parseMeasurement(json_input)
    measurements.setdefault(name, []).append(m)

  if baseline_name not in measurements:
    raise "No baseline benchmark found"

  variations = []
  for name, ms in measurements.iteritems():
    if name == baseline_name:
      continue
    for m, baseline in zip(ms, measurements[baseline_name]):
      assert(m.input_size == baseline.input_size)
      m.time - baseline.time
    variations.append(Variation(name, ms))

  del measurements[baseline_name]

  return variations

def drawPlot(variations):
  traces = []
  for variation in variations:
    input_sizes = map(lambda m : m.input_size, variation.measurements)
    times = map(lambda m: m.time, variation.measurements)
    line = dict( width = 3)

    trace = Scatter(x = input_sizes,
                    y = times,
                    mode = 'lines',
                    name = variation.name,
                    line = line
                    )
    traces.append(trace)

  data = plotly.graph_objs.Data(traces)
  layout = {}

  dtick_x = 40
  dtick_y = 1000
  if last_40 == True:
    dtick_x = 1
    dtick_y = 200

  layout['xaxis'] = dict(title='lhs_size',
                         autotick=False,
                         ticks='outside',
                         tick0=0,
                         dtick=dtick_x,
                         ticklen=8,
                         tickwidth=4,
                         tickcolor='#000')
  layout['yaxis'] = dict(title='ns',
                         autotick=False,
                         ticks='outside',
                         tick0=0,
                         dtick=dtick_y,
                         ticklen=8,
                         tickwidth=4,
                         tickcolor='#000')

  return plotly.offline.plot(
    dict(data=data, layout=layout),
    auto_open = True,
  )

if __name__ == "__main__":
  options_parser = argparse.ArgumentParser( \
        description='Comparing performance of different implementations.')

  options_parser.add_argument('--benchmarks_result',
                               dest='benchmarks_result',
                               required=True)
  options_parser.add_argument('--last_40', action='store_true', dest='last_40')
  options_parser.add_argument('--baseline_name',
                               dest='baseline_name',
                               default='baseline')
  options = options_parser.parse_args()
  last_40 = options.last_40
  baseline_name = options.baseline_name
  loaded_benchmarks = json.load(open(options.benchmarks_result))

  variations = parseJson(loaded_benchmarks, baseline_name)
  drawPlot(variations)
