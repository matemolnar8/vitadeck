import type { ReactNode } from "react";
import type { ReconcilerMetrics } from "./reconciler-metrics";
import { persistentReconcilerMetrics, VitadeckReactReconciler } from "./vitadeck-react-reconciler";
import { mutationReconcilerMetrics, VitadeckReactMutationReconciler } from "./vitadeck-react-reconciler-mutation";
import type { VitadeckContainer } from "./vitadeck-reconciler-shared";

export type ReconcilerId = "persistent" | "mutation";

type ManagedReconcilerInstance = typeof VitadeckReactReconciler;
type ManagedRoot = ReturnType<ManagedReconcilerInstance["createContainer"]>;

type ReconcilerConfig = {
  id: ReconcilerId;
  label: string;
  instance: ManagedReconcilerInstance;
  metrics: ReconcilerMetrics;
};

export type ReconcilerDescriptor = {
  id: ReconcilerId;
  label: string;
};

type ManagerOptions = {
  identifierPrefix?: string;
  initialActiveReconciler?: ReconcilerId;
  onError?: (error: unknown) => void;
};

const DEFAULT_IDENTIFIER_PREFIX = "vitadeck_react_";
const DEFAULT_ACTIVE_RECONCILER: ReconcilerId = "mutation";

const RECONCILER_CONFIG: Record<ReconcilerId, ReconcilerConfig> = {
  persistent: {
    id: "persistent",
    label: "Persistent",
    instance: VitadeckReactReconciler,
    metrics: persistentReconcilerMetrics,
  },
  mutation: {
    id: "mutation",
    label: "Mutation",
    instance: VitadeckReactMutationReconciler,
    metrics: mutationReconcilerMetrics,
  },
};

const reconcilerListMutable: ReconcilerDescriptor[] = [];
const reconcilerKeys = Object.keys(RECONCILER_CONFIG) as ReconcilerId[];
for (let i = 0; i < reconcilerKeys.length; i++) {
  const key = reconcilerKeys[i];
  if (!key) continue;
  const entry = RECONCILER_CONFIG[key];
  if (!entry) continue;
  reconcilerListMutable.push({ id: entry.id, label: entry.label });
}
const RECONCILER_LIST: ReadonlyArray<ReconcilerDescriptor> = Object.freeze(reconcilerListMutable);

let runtimeOptions: Required<Pick<ManagerOptions, "identifierPrefix" | "onError">> | null = null;
let initialized = false;
let activeReconciler: ReconcilerId = DEFAULT_ACTIVE_RECONCILER;
let currentElement: ReactNode | null = null;
type ReconcilerRuntimeState = ReconcilerConfig & {
  container: VitadeckContainer;
  root: ManagedRoot;
};
let currentState: ReconcilerRuntimeState | null = null;

function defaultOnError(error: unknown) {
  console.error("[ReconcilerManager] Recoverable error", error);
}

function ensureInitialized() {
  if (!initialized || !runtimeOptions || !currentState) {
    throw new Error("Reconciler manager not initialized. Call initializeReconcilerManager first.");
  }
}

function createState(config: ReconcilerConfig): ReconcilerRuntimeState {
  if (!runtimeOptions) {
    throw new Error("Reconciler manager runtime options not set.");
  }
  const container: VitadeckContainer = { children: [] };
  const root = config.instance.createContainer(
    container,
    0,
    null,
    true,
    null,
    `${runtimeOptions.identifierPrefix}${config.id}_`,
    runtimeOptions.onError,
    null,
  );
  return {
    ...config,
    container,
    root,
  };
}

function renderWithReconciler(state: ReconcilerRuntimeState, element: ReactNode) {
  state.instance.updateContainer(element, state.root);
}

export function initializeReconcilerManager(options: ManagerOptions = {}) {
  if (initialized) return;

  runtimeOptions = {
    identifierPrefix: options.identifierPrefix ?? DEFAULT_IDENTIFIER_PREFIX,
    onError: options.onError ?? defaultOnError,
  };

  activeReconciler = options.initialActiveReconciler ?? DEFAULT_ACTIVE_RECONCILER;
  currentState = createState(RECONCILER_CONFIG[activeReconciler]);
  initialized = true;
}

export function getAvailableReconcilers(): ReconcilerDescriptor[] {
  return RECONCILER_LIST.slice();
}

export function getActiveReconciler(): ReconcilerDescriptor {
  ensureInitialized();
  const { id, label } = RECONCILER_CONFIG[activeReconciler];
  return { id, label };
}

export function setActiveReconciler(nextId: ReconcilerId) {
  ensureInitialized();
  if (!(nextId in RECONCILER_CONFIG)) {
    throw new Error(`Unknown reconciler: ${nextId}`);
  }
  if (activeReconciler === nextId) return;

  if (currentState) {
    currentState.instance.updateContainer(null, currentState.root);
  }

  activeReconciler = nextId;
  currentState = createState(RECONCILER_CONFIG[nextId]);
  if (currentElement) {
    renderWithReconciler(currentState, currentElement);
  }
}

export function updateTree(element: ReactNode) {
  ensureInitialized();
  currentElement = element;
  if (currentState) {
    renderWithReconciler(currentState, element);
  }
}

export function getReconcilerMetrics(id?: ReconcilerId): ReconcilerMetrics {
  ensureInitialized();
  const targetId = id ?? activeReconciler;
  return RECONCILER_CONFIG[targetId].metrics;
}

export function resetMetrics(id?: ReconcilerId) {
  ensureInitialized();
  if (id) {
    RECONCILER_CONFIG[id].metrics.reset();
    return;
  }
  const configKeys = Object.keys(RECONCILER_CONFIG) as ReconcilerId[];
  for (let i = 0; i < configKeys.length; i++) {
    const key = configKeys[i];
    if (!key) continue;
    const config = RECONCILER_CONFIG[key];
    if (config) {
      config.metrics.reset();
    }
  }
}

export function getCurrentElement(): ReactNode | null {
  return currentElement;
}

export function isReconcilerManagerInitialized(): boolean {
  return initialized;
}
