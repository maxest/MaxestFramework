import torch
import numpy as np
from torch.autograd import Variable
import torch_layers_dumper


torch.backends.cudnn.deterministic = True
torch.manual_seed(42)
torch.cuda.manual_seed(42)
np.random.seed(42)


layer1 = torch.nn.Linear(1, 24, True)
layer2 = torch.nn.Linear(24, 24, False)
layer3 = torch.nn.Linear(24, 1, True)

model = torch.nn.Sequential(
	layer1,
	torch.nn.Sigmoid(),
	layer2,
	torch.nn.Sigmoid(),
	layer3,
	torch.nn.Sigmoid(),
)


x = [ [0.0], [0.1], [0.2], [0.3], [0.4], [0.5], [0.6], [0.7], [0.8], [0.9] ]
x = Variable(torch.FloatTensor(x))
y = [ [0.0], [0.1], [0.2], [0.3], [0.9], [0.9], [0.3], [0.2], [0.1], [0.0] ]
y = Variable(torch.FloatTensor(y))

learning_rate = 0.01
optim = torch.optim.Adam(model.parameters(), lr=learning_rate)

for i in range(10000):
	p = np.random.permutation(10)

	x2 = x[ p[0:10].tolist() ]
	y2 = y[ p[0:10].tolist() ]
	y2_hat = model.forward(x2)
	
	err = y2 - y2_hat
	err = err * err
	err = torch.sum(err)
	err /= 10.0

	optim.zero_grad()
	err.backward()
	optim.step()
	

torch_layers_dumper.DumpSequential("model.txt", model)

	
y_hat = model(x)
print("Real\n", y)
print("Predicted\n", y_hat)
